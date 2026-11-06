#include "mochottlongpost.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonArray>
#include <QRegularExpression>
#include <QSet>
#include <QUrl>

using namespace AtProtocolType;

namespace MochottLongPost {

namespace {

const QString MOCHOTT_URL = QStringLiteral("https://mochott.site");
const QString BSKY_URL = QStringLiteral("https://bsky.app");

// 記事の末尾の添付と照合するための、ポストのembedの情報
struct PostEmbeds
{
    QSet<QString> image_cids;
    QSet<QString> urls;
};

bool isHttpUrl(const QString &url)
{
    const QUrl parsed(url);
    return parsed.isValid() && !parsed.host().isEmpty()
            && (parsed.scheme() == QStringLiteral("http")
                || parsed.scheme() == QStringLiteral("https"));
}

QString recordUriToBskyUrl(const QString &uri)
{
    // at://{did}/{collection}/{rkey}
    const QStringList items = uri.split("/");
    if (items.count() != 5 || !uri.startsWith("at://"))
        return QString();
    const QString &did = items.at(2);
    const QString &collection = items.at(3);
    const QString &rkey = items.at(4);
    if (collection == QStringLiteral("app.bsky.feed.post")) {
        return QString("%1/profile/%2/post/%3").arg(BSKY_URL, did, rkey);
    } else if (collection == QStringLiteral("app.bsky.feed.generator")) {
        return QString("%1/profile/%2/feed/%3").arg(BSKY_URL, did, rkey);
    } else if (collection == QStringLiteral("app.bsky.graph.list")) {
        return QString("%1/profile/%2/lists/%3").arg(BSKY_URL, did, rkey);
    } else if (collection == QStringLiteral("app.bsky.graph.starterpack")) {
        return QString("%1/starter-pack/%2/%3").arg(BSKY_URL, did, rkey);
    }
    return QString();
}

void appendImages(const AppBskyEmbedImages::Main &images, PostEmbeds &embeds)
{
    for (const auto &image : images.images) {
        embeds.image_cids.insert(image.image.cid);
    }
}

void appendGallery(const AppBskyEmbedGallery::Main &gallery, PostEmbeds &embeds)
{
    for (const auto &image : gallery.items_Image) {
        embeds.image_cids.insert(image.image.cid);
    }
}

void appendUrl(const QString &url, PostEmbeds &embeds)
{
    if (!url.isEmpty()) {
        embeds.urls.insert(url);
    }
}

PostEmbeds collectPostEmbeds(const AppBskyFeedPost::Main &record)
{
    PostEmbeds embeds;
    appendImages(record.embed_AppBskyEmbedImages_Main, embeds);
    appendGallery(record.embed_AppBskyEmbedGallery_Main, embeds);
    appendUrl(record.embed_AppBskyEmbedExternal_Main.external.uri, embeds);
    appendUrl(recordUriToBskyUrl(record.embed_AppBskyEmbedRecord_Main.record.uri), embeds);

    const auto &with_media = record.embed_AppBskyEmbedRecordWithMedia_Main;
    appendImages(with_media.media_AppBskyEmbedImages_Main, embeds);
    appendGallery(with_media.media_AppBskyEmbedGallery_Main, embeds);
    appendUrl(with_media.media_AppBskyEmbedExternal_Main.external.uri, embeds);
    if (with_media.record) {
        appendUrl(recordUriToBskyUrl(with_media.record->record.uri), embeds);
    }
    return embeds;
}

QString makeLink(const QString &url, const QString &display_html)
{
    return QString("<a href=\"%1\">%2</a>").arg(url.toHtmlEscaped(), display_html);
}

// 表にないものがあるときはfalseを返す
bool parseParagraph(const QJsonObject &paragraph, QString &html, QString &plain)
{
    for (const auto value : paragraph.value("content").toArray()) {
        const QJsonObject node = value.toObject();
        const QString type = node.value("type").toString();
        if (type == QStringLiteral("hardBreak")) {
            html += QStringLiteral("<br/>");
            plain += QStringLiteral("\n");
        } else if (type == QStringLiteral("text")) {
            const QString text = node.value("text").toString();
            QString text_html = LexiconsTypeUnknown::replaceCrToBr(text, true);
            for (const auto mark_value : node.value("marks").toArray()) {
                const QJsonObject mark = mark_value.toObject();
                if (mark.value("type").toString() != QStringLiteral("link"))
                    return false;
                const QString href = mark.value("attrs").toObject().value("href").toString();
                if (isHttpUrl(href)) {
                    text_html = makeLink(href, text_html);
                }
            }
            html += text_html;
            plain += text;
        } else {
            return false;
        }
    }
    return true;
}

}

Reference parseReference(const AppBskyFeedPost::Main &record, const QString &author_did)
{
    static const QRegularExpression regex(
            QStringLiteral("^at://(did:[a-z]+:[a-zA-Z0-9._:%-]+)/site\\.mochott\\.article/"
                           "([a-zA-Z0-9._~:-]{1,512})$"));

    Reference reference;
    const auto &ref = record.site_mochott_longPost;
    if (ref.uri.isEmpty() || ref.cid.isEmpty())
        return reference;
    const QRegularExpressionMatch match = regex.match(ref.uri);
    // 他人の記事を自分の全文のように見せることを防ぐ
    if (!match.hasMatch() || match.captured(1) != author_did)
        return reference;
    reference.uri = ref.uri;
    reference.cid = ref.cid;
    reference.rkey = match.captured(2);
    return reference;
}

Opening parseOpening(const AppBskyFeedPost::Main &record, const QString &rkey)
{
    Opening opening;
    opening.text = record.text;
    opening.facets = record.facets;

    // 文言は投稿者の言語で変わるのでfacetで判断する
    const QByteArray bytes = record.text.toUtf8();
    const QString prefix = MOCHOTT_URL + QStringLiteral("/");
    const QString suffix = QStringLiteral("/") + rkey;
    for (const auto &facet : record.facets) {
        if (facet.index.byteEnd != bytes.length() || facet.index.byteStart < 0
            || facet.index.byteStart > facet.index.byteEnd) {
            continue;
        }
        for (const auto &link : facet.features_Link) {
            if (link.uri.startsWith(prefix) && link.uri.endsWith(suffix)) {
                opening.article_url = link.uri;
                break;
            }
        }
        if (!opening.article_url.isEmpty()) {
            const int start = facet.index.byteStart;
            opening.text = QString::fromUtf8(bytes.left(start));
            opening.facets.clear();
            for (const auto &other : record.facets) {
                if (other.index.byteEnd <= start) {
                    opening.facets.append(other);
                }
            }
            break;
        }
    }
    return opening;
}

Article parseArticle(const QJsonObject &article, const Reference &reference,
                     const AppBskyFeedPost::Main &record, const QString &author_did)
{
    Article result;
    if (!reference.isValid() || article.value("uri").toString() != reference.uri)
        return result;
    const QJsonObject doc = article.value("contentJSON").toObject();
    if (doc.value("type").toString() != QStringLiteral("doc"))
        return result;

    const PostEmbeds embeds = collectPostEmbeds(record);
    const QRegularExpression image_regex(QStringLiteral("^/api/image/%1/([a-zA-Z0-9]+)$")
                                                 .arg(QRegularExpression::escape(author_did)));
    QStringList html_lines;
    QStringList plain_lines;
    QStringList link_html_lines;
    QStringList link_plain_lines;
    QSet<QString> used_cids;
    bool has_all_ratios = true;

    for (const auto value : doc.value("content").toArray()) {
        const QJsonObject node = value.toObject();
        const QString type = node.value("type").toString();
        const QJsonObject attrs = node.value("attrs").toObject();
        if (type == QStringLiteral("paragraph")) {
            QString html;
            QString plain;
            if (!parseParagraph(node, html, plain))
                return result;
            html_lines.append(html);
            plain_lines.append(plain);
        } else if (type == QStringLiteral("image")) {
            const QString src = attrs.value("src").toString();
            const QRegularExpressionMatch match = image_regex.match(src);
            if (!match.hasMatch())
                continue;
            const QString cid = match.captured(1);
            if (embeds.image_cids.contains(cid) || used_cids.contains(cid))
                continue;
            used_cids.insert(cid);
            result.images.append(MOCHOTT_URL + src);
            result.images_alt.append(attrs.value("alt").toString());
            const int width = attrs.value("width").toInt();
            const int height = attrs.value("height").toInt();
            if (width > 0 && height > 0) {
                result.images_ratio.append(
                        QString::number(static_cast<double>(height) / static_cast<double>(width)));
            } else {
                has_all_ratios = false;
            }
        } else if (type == QStringLiteral("linkCard") || type == QStringLiteral("embed")) {
            QString url;
            QString title;
            if (type == QStringLiteral("linkCard")) {
                url = attrs.value("url").toString();
                title = attrs.value("title").toString();
                if (!isHttpUrl(url))
                    continue;
            } else {
                url = attrs.value("src").toString();
                if (attrs.value("service").toString() != QStringLiteral("bluesky")
                    || !url.startsWith(BSKY_URL + QStringLiteral("/"))) {
                    continue;
                }
            }
            if (embeds.urls.contains(url))
                continue;
            const QString display = title.isEmpty() ? url : title;
            link_html_lines.append(makeLink(url, display.toHtmlEscaped()));
            link_plain_lines.append(title.isEmpty() ? url : QString("%1 %2").arg(title, url));
        } else {
            return result;
        }
    }

    // 末尾の空の段落は表示しない
    while (!plain_lines.isEmpty() && plain_lines.last().trimmed().isEmpty()) {
        plain_lines.removeLast();
        html_lines.removeLast();
    }
    html_lines.append(link_html_lines);
    plain_lines.append(link_plain_lines);

    if (html_lines.isEmpty())
        return result;

    if (!has_all_ratios) {
        result.images_ratio.clear();
    }
    result.text = html_lines.join(QStringLiteral("<br/>"));
    result.text_plain = plain_lines.join(QStringLiteral("\n"));
    result.edited = article.value("cid").toString() != reference.cid;
    result.valid = true;
    return result;
}

QString makeArticleUrl(const QString &handle, const QString &rkey)
{
    return QString("%1/%2/%3").arg(MOCHOTT_URL, handle, rkey);
}

}
