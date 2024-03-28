#ifndef LEXICONS_FUNC_UNKNOWN_CPP
#define LEXICONS_FUNC_UNKNOWN_CPP

#include "lexicons_func.h"
#include "lexicons_func_unknown.h"

namespace AtProtocolType {
namespace LexiconsTypeUnknown {

void copyUnknown(const QJsonObject &src, QVariant &dest)
{
    if (src.isEmpty())
        return;

    QString type = src.value("$type").toString();
    if (type == QStringLiteral("app.bsky.feed.post")) {
        AppBskyFeedPost::Main record;
        AppBskyFeedPost::copyMain(src, record);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        dest.setValue<AppBskyFeedPost::Main>(record);
#else
        dest.setValue(record);
#endif
    } else if (type == QStringLiteral("app.bsky.feed.like")) {
        AppBskyFeedLike::Main record;
        AppBskyFeedLike::copyMain(src, record);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        dest.setValue<AppBskyFeedLike::Main>(record);
#else
        dest.setValue(record);
#endif

    } else if (type == QStringLiteral("app.bsky.feed.repost")) {
        AppBskyFeedRepost::Main record;
        AppBskyFeedRepost::copyMain(src, record);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        dest.setValue<AppBskyFeedRepost::Main>(record);
#else
        dest.setValue(record);
#endif
    } else if (type == QStringLiteral("app.bsky.graph.listitem")) {
        AppBskyGraphListitem::Main record;
        AppBskyGraphListitem::copyMain(src, record);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        dest.setValue<AppBskyGraphListitem::Main>(record);
#else
        dest.setValue(record);
#endif
    } else if (type == QStringLiteral("app.bsky.actor.profile")) {
        AppBskyActorProfile::Main record;
        AppBskyActorProfile::copyMain(src, record);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        dest.setValue<AppBskyActorProfile::Main>(record);
#else
        dest.setValue(record);
#endif
    } else if (type == QStringLiteral("app.bsky.graph.list")) {
        AppBskyGraphList::Main record;
        AppBskyGraphList::copyMain(src, record);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        dest.setValue<AppBskyGraphList::Main>(record);
#else
        dest.setValue(record);
#endif
    } else if (type == QStringLiteral("app.bsky.feed.threadgate")) {
        AppBskyFeedThreadgate::Main record;
        AppBskyFeedThreadgate::copyMain(src, record);
        dest.setValue(record);
    }
}

void copyBlob(const QJsonObject &src, Blob &dest)
{
    if (src.isEmpty())
        return;
    if (src.value("$type").toString() != "blob")
        return;

    dest.cid = src.value("ref").toObject().value("$link").toString();
    dest.alt = src.value("alt").toString();
    dest.mimeType = src.value("mimeType").toString();
    dest.size = src.value("size").toInt();
}

QStringList copyImagesFromPostView(const AppBskyFeedDefs::PostView &post, const CopyImageType type)
{
    if (post.embed_type == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedImages_View) {
        QStringList images;
        for (const auto &image : post.embed_AppBskyEmbedImages_View.images) {
            if (type == CopyImageType::Thumb)
                images.append(image.thumb);
            else if (type == CopyImageType::FullSize)
                images.append(image.fullsize);
            else if (type == CopyImageType::Alt)
                images.append(image.alt);
        }
        return images;
    } else if (post.embed_type
                       == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedRecordWithMedia_View
               && post.embed_AppBskyEmbedRecordWithMedia_View.media_type
                       == AppBskyEmbedRecordWithMedia::ViewMediaType::
                               media_AppBskyEmbedImages_View) {
        QStringList images;
        for (const auto &image :
             post.embed_AppBskyEmbedRecordWithMedia_View.media_AppBskyEmbedImages_View.images) {
            if (type == CopyImageType::Thumb)
                images.append(image.thumb);
            else if (type == CopyImageType::FullSize)
                images.append(image.fullsize);
            else if (type == CopyImageType::Alt)
                images.append(image.alt);
        }
        return images;
    } else {
        return QStringList();
    }
}

QStringList copyImagesFromRecord(const AppBskyEmbedRecord::ViewRecord &record,
                                 const CopyImageType type)
{
    // unionの配列で複数種類を混ぜられる
    QStringList images;
    for (const auto &view : record.embeds_AppBskyEmbedImages_View) {
        for (const auto &image : view.images) {
            if (type == CopyImageType::Thumb)
                images.append(image.thumb);
            else if (type == CopyImageType::FullSize)
                images.append(image.fullsize);
            else if (type == CopyImageType::Alt)
                images.append(image.alt);
        }
    }
    for (const auto &view : record.embeds_AppBskyEmbedRecordWithMedia_View) {
        if (view.media_type
            == AppBskyEmbedRecordWithMedia::ViewMediaType::media_AppBskyEmbedImages_View) {
            for (const auto &image : view.media_AppBskyEmbedImages_View.images) {
                if (type == CopyImageType::Thumb)
                    images.append(image.thumb);
                else if (type == CopyImageType::FullSize)
                    images.append(image.fullsize);
                else if (type == CopyImageType::Alt)
                    images.append(image.alt);
            }
        }
    }
    return images;
}

QStringList copyTagsFromFacets(const QList<AppBskyRichtextFacet::Main> &facets)
{
    QStringList tags;
    for (const auto &facet : facets) {
        for (const auto &tag : facet.features_Tag) {
            if (!tag.tag.isEmpty()) {
                tags.append(tag.tag);
            }
        }
    }
    return tags;
}

bool checkPartialMatchLanguage(const QStringList &langs)
{
    for (const auto &lang : langs) {
        QLocale locale(lang);
        if (locale.language() == QLocale::Japanese) {
            return true;
        }
    }
    return false;
}

QString copyRecordText(const QVariant &value)
{
    const AppBskyFeedPost::Main record =
            LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(value);
    if (record.facets.isEmpty()) {
        return QString(record.text).toHtmlEscaped().replace("\n", "<br/>");
    } else {
        QByteArray text_ba = record.text.toUtf8();
        QString text;
        int pos_start = 0;
        int pos_end = 0;
        int pos_prev_end = 0;
        QList<AppBskyRichtextFacet::Main> facets = record.facets;
        for (int i = 0; i < facets.length() - 1; i++) {
            for (int j = i + 1; j < facets.length(); j++) {
                if (facets.at(i).index.byteStart > facets.at(j).index.byteStart) {
                    facets.swapItemsAt(i, j);
                }
            }
        }
        for (const auto &part : facets) {
            pos_start = part.index.byteStart;
            pos_end = part.index.byteEnd;

            // 0 : [a]b(c)
            // 1 : [(abc)]
            // 2 : [a(bc)]
            // 3 : [a(b)c]
            // 4 : [a(b]c)

            if (pos_start < pos_prev_end) {
                // 前回の終了位置より開始が前（つまり、互い違いになっている -> [あ(い]う)の状態）
                if (pos_end <= pos_prev_end) {
                    // 1 : [(abc)]
                    // 2 : [a(bc)]
                    // 3 : [a(b)c]
                } else {
                    // 4 : [a(b]c)
                    text += QString(text_ba.mid(pos_prev_end, pos_end - pos_prev_end))
                                    .toHtmlEscaped()
                                    .replace("\n", "<br/>");
                    pos_prev_end = pos_end;
                }
            } else {
                // 0 : [a]b(c)
                if (pos_start > pos_prev_end) {
                    text += QString(text_ba.mid(pos_prev_end, pos_start - pos_prev_end))
                                    .toHtmlEscaped()
                                    .replace("\n", "<br/>");
                }
                QString display_url =
                        QString::fromUtf8(text_ba.mid(pos_start, pos_end - pos_start));
                if (!part.features_Link.isEmpty()) {
                    text += QString("<a href=\"%1\">%2</a>")
                                    .arg(part.features_Link.first().uri, display_url);
                } else if (!part.features_Mention.isEmpty()) {
                    text += QString("<a href=\"%1\">%2</a>")
                                    .arg(part.features_Mention.first().did, display_url);
                } else if (!part.features_Tag.isEmpty()) {
                    text += QString("<a href=\"search://%1\">%2</a>")
                                    .arg(part.features_Tag.first().tag, display_url);
                } else {
                    text += QString(text_ba.mid(pos_start, pos_end - pos_start))
                                    .toHtmlEscaped()
                                    .replace("\n", "<br/>");
                }
                pos_prev_end = pos_end;
            }
        }
        if (pos_prev_end < (text_ba.length() - 1)) {
            text += QString(text_ba.mid(pos_prev_end)).toHtmlEscaped().replace("\n", "<br/>");
        }

        return text;
    }
}

QString formatDateTime(const QString &value, const bool is_long)
{
    if (is_long)
        return QDateTime::fromString(value, Qt::ISODateWithMs)
                .toLocalTime()
                .toString("yyyy/MM/dd hh:mm:ss");
    else
        return QDateTime::fromString(value, Qt::ISODateWithMs)
                .toLocalTime()
                .toString("MM/dd hh:mm");
}

}
}

#endif // LEXICONS_FUNC_UNKNOWN_CPP
