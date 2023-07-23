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
    }
}

QStringList copyImagesFromPostView(const AppBskyFeedDefs::PostView &post, const bool thumb)
{
    if (post.embed_type == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedImages_View) {
        QStringList images;
        for (const auto &image : post.embed_AppBskyEmbedImages_View.images) {
            if (thumb)
                images.append(image.thumb);
            else
                images.append(image.fullsize);
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
            if (thumb)
                images.append(image.thumb);
            else
                images.append(image.fullsize);
        }
        return images;
    } else {
        return QStringList();
    }
}

QStringList copyImagesFromRecord(const AppBskyEmbedRecord::ViewRecord &record, const bool thumb)
{
    if (record.embeds_type
        == AppBskyEmbedRecord::ViewRecordEmbedsType::embeds_AppBskyEmbedImages_View) {
        QStringList images;
        for (const auto &view : record.embeds_AppBskyEmbedImages_View) {
            for (const auto &image : view.images) {
                if (thumb)
                    images.append(image.thumb);
                else
                    images.append(image.fullsize);
            }
        }
        return images;
    } else {
        return QStringList();
    }
}

}
}

#endif // LEXICONS_FUNC_UNKNOWN_CPP
