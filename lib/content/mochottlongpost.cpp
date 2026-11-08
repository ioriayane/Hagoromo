#include "mochottlongpost.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "util/qstringex.h"

#include <QJsonArray>
#include <algorithm>
#include <QRegularExpression>
#include <QSet>
#include <QTextBoundaryFinder>
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

// ---- 書く側 ----

namespace {

const QString ELLIPSIS = QString::fromUtf8("…");

int countGraphemes(const QString &text)
{
    // 投稿ダイアログの文字数の数え方と合わせる
    return QStringEx(text).length();
}

// 書記素の境界で上限に収まるように切る
QString truncateText(const QString &text, const int max_graphemes, const int max_bytes)
{
    if (max_graphemes <= 0 || max_bytes <= 0)
        return QString();
    if (countGraphemes(text) <= max_graphemes && text.toUtf8().size() <= max_bytes)
        return text;

    QTextBoundaryFinder finder(QTextBoundaryFinder::Grapheme, text);
    int prev = 0;
    int graphemes = 0;
    int bytes = 0;
    while (finder.toNextBoundary() >= 0) {
        const int pos = finder.position();
        const QString part = text.mid(prev, pos - prev);
        graphemes += countGraphemes(part);
        bytes += part.toUtf8().size();
        if (graphemes > max_graphemes || bytes > max_bytes)
            break;
        prev = pos;
    }
    return text.left(prev);
}

QJsonObject makeBlobJson(const AtProtocolType::Blob &blob)
{
    QJsonObject ref;
    ref.insert("$link", blob.cid);
    QJsonObject json;
    json.insert("$type", "blob");
    json.insert("ref", ref);
    json.insert("mimeType", blob.mimeType);
    json.insert("size", blob.size);
    return json;
}

QString facetHref(const AtProtocolType::AppBskyRichtextFacet::Main &facet)
{
    if (!facet.features_Link.isEmpty() && isHttpUrl(facet.features_Link.first().uri)) {
        return facet.features_Link.first().uri;
    } else if (!facet.features_Mention.isEmpty()
               && facet.features_Mention.first().did.startsWith("did:")) {
        return QString("%1/profile/%2").arg(BSKY_URL, facet.features_Mention.first().did);
    }
    // タグは記事ではリンクにできない
    return QString();
}

void appendTextNode(const QByteArray &bytes, const QString &href, QJsonArray &dest)
{
    const QString text = QString::fromUtf8(bytes);
    if (text.isEmpty())
        return;
    QJsonObject node;
    node.insert("type", "text");
    node.insert("text", text);
    if (!href.isEmpty()) {
        QJsonObject attrs;
        attrs.insert("href", href);
        QJsonObject mark;
        mark.insert("type", "link");
        mark.insert("attrs", attrs);
        node.insert("marks", QJsonArray() << mark);
    }
    dest.append(node);
}

// 1行を1つの段落にする(空行は空の段落)
QJsonArray makeParagraphs(const QString &text,
                          const QList<AtProtocolType::AppBskyRichtextFacet::Main> &facets)
{
    QList<AtProtocolType::AppBskyRichtextFacet::Main> sorted = facets;
    std::sort(sorted.begin(), sorted.end(),
              [](const AtProtocolType::AppBskyRichtextFacet::Main &a,
                 const AtProtocolType::AppBskyRichtextFacet::Main &b) {
                  return a.index.byteStart < b.index.byteStart;
              });

    const QByteArray bytes = text.toUtf8();
    QJsonArray paragraphs;
    int line_start = 0;
    while (line_start <= bytes.size()) {
        int line_end = bytes.indexOf('\n', line_start);
        if (line_end < 0) {
            line_end = bytes.size();
        }
        QJsonArray inlines;
        int pos = line_start;
        for (const auto &facet : sorted) {
            const int start = facet.index.byteStart;
            const int end = facet.index.byteEnd;
            if (start < pos || end > line_end || start >= end)
                continue;
            appendTextNode(bytes.mid(pos, start - pos), QString(), inlines);
            appendTextNode(bytes.mid(start, end - start), facetHref(facet), inlines);
            pos = end;
        }
        appendTextNode(bytes.mid(pos, line_end - pos), QString(), inlines);

        QJsonObject paragraph;
        paragraph.insert("type", "paragraph");
        if (!inlines.isEmpty()) {
            paragraph.insert("content", inlines);
        }
        paragraphs.append(paragraph);
        line_start = line_end + 1;
    }
    // 末尾の空の段落は書かない
    while (!paragraphs.isEmpty() && !paragraphs.last().toObject().contains("content")) {
        paragraphs.removeLast();
    }
    return paragraphs;
}

QString makeTitle(const QString &text)
{
    QString title;
    for (const auto &line : text.split('\n')) {
        title = line.trimmed();
        if (!title.isEmpty())
            break;
    }
    return truncateText(title, 500, 5000);
}

QString makeImageSrc(const QString &did, const QString &cid)
{
    return QString("/api/image/%1/%2").arg(did, cid);
}

void appendArticleBlob(const AtProtocolType::Blob &blob, const QString &name, QJsonArray &blobs)
{
    QJsonObject json;
    json.insert("blobref", makeBlobJson(blob));
    json.insert("name", truncateText(name.isEmpty() ? blob.cid : name, 500, 500));
    blobs.append(json);
}

}

PostOpening makeOpening(const QString &text,
                        const QList<AtProtocolType::AppBskyRichtextFacet::Main> &facets,
                        const QString &more_label, const QString &article_url)
{
    const QString tail = ELLIPSIS + more_label;
    const QByteArray bytes = text.toUtf8();

    int cut = truncateText(text, POST_MAX_GRAPHEMES - countGraphemes(tail),
                           POST_MAX_BYTES - tail.toUtf8().size())
                      .toUtf8()
                      .size();
    // リンク・メンション・タグの途中で切らない
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto &facet : facets) {
            if (facet.index.byteStart < cut && facet.index.byteEnd > cut) {
                cut = qMax(0, facet.index.byteStart);
                changed = true;
            }
        }
    }
    QString body = QString::fromUtf8(bytes.left(cut));
    while (!body.isEmpty() && body.at(body.length() - 1).isSpace()) {
        body.chop(1);
    }

    PostOpening opening;
    opening.body = body;
    opening.text = body + tail;
    const int body_bytes = body.toUtf8().size();
    for (const auto &facet : facets) {
        if (facet.index.byteStart >= 0 && facet.index.byteEnd <= body_bytes) {
            opening.facets.append(facet);
        }
    }
    AtProtocolType::AppBskyRichtextFacet::Main link;
    link.index.byteStart = (body + ELLIPSIS).toUtf8().size();
    link.index.byteEnd = opening.text.toUtf8().size();
    link.features_type = AtProtocolType::AppBskyRichtextFacet::MainFeaturesType::features_Link;
    AtProtocolType::AppBskyRichtextFacet::Link feature;
    feature.uri = article_url;
    link.features_Link.append(feature);
    opening.facets.append(link);
    return opening;
}

QJsonObject makeArticleRecord(const ArticleSource &source)
{
    QString text = source.text;
    text.remove('\r');
    text = truncateText(text, ARTICLE_MAX_BYTES, ARTICLE_MAX_BYTES);

    // 並びは「段落 → image → linkCard → embed」
    QJsonArray content = makeParagraphs(text, source.facets);
    QJsonArray blobs;
    const auto &data = source.attachments;
    QJsonArray link_cards;
    if (!data.external_link_uri.isEmpty()) {
        // リンクカードのときの画像はサムネイル
        if (isHttpUrl(data.external_link_uri)) {
            QJsonObject attrs;
            attrs.insert("url", data.external_link_uri);
            if (!data.external_link_title.isEmpty()) {
                attrs.insert("title", data.external_link_title.left(300));
            }
            if (!data.external_link_description.isEmpty()) {
                attrs.insert("description", data.external_link_description.left(1000));
            }
            if (!data.embed_image_blobs.isEmpty()) {
                const auto &thumb = data.embed_image_blobs.first();
                attrs.insert("image", makeImageSrc(source.did, thumb.cid));
                appendArticleBlob(thumb, data.external_link_title, blobs);
            }
            QJsonObject node;
            node.insert("type", "linkCard");
            node.insert("attrs", attrs);
            link_cards.append(node);
        }
    } else {
        for (const auto &blob : data.embed_image_blobs) {
            QJsonObject attrs;
            attrs.insert("src", makeImageSrc(source.did, blob.cid));
            if (!blob.alt.isEmpty()) {
                attrs.insert("alt", blob.alt);
            }
            if (blob.aspect_ratio.width() > 0 && blob.aspect_ratio.height() > 0) {
                attrs.insert("width", blob.aspect_ratio.width());
                attrs.insert("height", blob.aspect_ratio.height());
            }
            QJsonObject node;
            node.insert("type", "image");
            node.insert("attrs", attrs);
            content.append(node);
            appendArticleBlob(blob, blob.alt, blobs);
        }
    }
    if (!data.feed_generator_link_uri.isEmpty()) {
        // フィード・リストのカード
        const QString url = recordUriToBskyUrl(data.feed_generator_link_uri);
        if (!url.isEmpty()) {
            QJsonObject attrs;
            attrs.insert("url", url);
            QJsonObject node;
            node.insert("type", "linkCard");
            node.insert("attrs", attrs);
            link_cards.append(node);
        }
    }
    for (const auto &node : link_cards) {
        content.append(node);
    }
    const QStringList quote_items = data.embed_quote.uri.split("/");
    if (quote_items.count() == 5 && quote_items.at(2).startsWith("did:")
        && quote_items.at(3) == QStringLiteral("app.bsky.feed.post")) {
        QJsonObject attrs;
        attrs.insert("src", recordUriToBskyUrl(data.embed_quote.uri));
        attrs.insert("service", "bluesky");
        attrs.insert("embedUrl",
                     QString("https://embed.bsky.app/embed/%1/app.bsky.feed.post/%2")
                             .arg(quote_items.at(2), quote_items.at(4)));
        QJsonObject node;
        node.insert("type", "embed");
        node.insert("attrs", attrs);
        content.append(node);
    }

    QJsonObject doc;
    doc.insert("type", "doc");
    doc.insert("content", content);

    const QString title = makeTitle(text);
    QString description = source.description.isEmpty() ? title : source.description;
    description = truncateText(description, 3000, 30000);

    QJsonObject record;
    record.insert("$type", "site.mochott.article");
    record.insert("profile", source.profile);
    record.insert("title", title);
    record.insert("createdAt", source.created_at);
    record.insert("updatedAt", source.created_at);
    record.insert("path", QStringLiteral("/") + source.rkey);
    record.insert("description", description);
    record.insert("textContent", text);
    record.insert("category", "chotto");
    record.insert("content", doc);
    if (!blobs.isEmpty()) {
        record.insert("blobs", blobs);
    }
    QJsonArray label_values;
    const QStringList allowed_labels = QStringList() << "sexual"
                                                     << "nudity"
                                                     << "graphic-media"
                                                     << "porn";
    for (const auto &label : data.self_labels) {
        if (allowed_labels.contains(label) && label_values.count() < 10) {
            QJsonObject value;
            value.insert("val", label);
            label_values.append(value);
        }
    }
    if (!label_values.isEmpty()) {
        QJsonObject labels;
        labels.insert("$type", "com.atproto.label.defs#selfLabels");
        labels.insert("values", label_values);
        record.insert("labels", labels);
    }
    record.insert("via", ARTICLE_VIA);
    return record;
}

QJsonObject makeDocumentRecord(const QJsonObject &article, const QString &site,
                               const AtProtocolType::ComAtprotoRepoStrongRef::Main &post)
{
    QJsonObject post_ref;
    post_ref.insert("uri", post.uri);
    post_ref.insert("cid", post.cid);

    QJsonObject record;
    record.insert("$type", "site.standard.document");
    record.insert("site", site);
    record.insert("title", article.value("title").toString());
    record.insert("publishedAt", article.value("createdAt").toString());
    record.insert("path", article.value("path").toString());
    record.insert("description", article.value("description").toString());
    record.insert("textContent", article.value("textContent").toString());
    record.insert("bskyPostRef", post_ref);
    return record;
}

}
