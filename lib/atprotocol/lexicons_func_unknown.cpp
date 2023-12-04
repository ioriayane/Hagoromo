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

}
}

#endif // LEXICONS_FUNC_UNKNOWN_CPP
