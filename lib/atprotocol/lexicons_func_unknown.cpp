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
        dest.setValue<AppBskyFeedPost::Main>(record);

    } else if (type == QStringLiteral("app.bsky.feed.like")) {
        AppBskyFeedLike::Main record;
        AppBskyFeedLike::copyMain(src, record);
        ComAtprotoRepoStrongRef::copyMain(src.value("subject").toObject(), record.subject);
        dest.setValue<AppBskyFeedLike::Main>(record);

    } else if (type == QStringLiteral("app.bsky.feed.repost")) {
        AppBskyFeedRepost::Main record;
        AppBskyFeedRepost::copyMain(src, record);
        ComAtprotoRepoStrongRef::copyMain(src.value("subject").toObject(), record.subject);
        dest.setValue<AppBskyFeedRepost::Main>(record);
    }
}

QString copyImagesFromPostView(const AppBskyFeedDefs::PostView &post, const bool thumb)
{
    if (post.embed_type == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedImages_View) {
        QString images;
        for (const auto &image : post.embed_AppBskyEmbedImages_View.images) {
            if (!images.isEmpty())
                images.append("\n");
            if (thumb)
                images.append(image.thumb);
            else
                images.append(image.fullsize);
        }
        return images;
    } else {
        return QString();
    }
}

QString copyImagesFromRecord(const AppBskyEmbedRecord::ViewRecord &record, const bool thumb)
{
    if (record.embeds_type
        == AppBskyEmbedRecord::ViewRecordEmbedsType::embeds_AppBskyEmbedImages_View) {
        QString images;
        for (const auto &view : record.embeds_AppBskyEmbedImages_View) {
            for (const auto &image : view.images) {
                if (!images.isEmpty())
                    images.append("\n");
                if (thumb)
                    images.append(image.thumb);
                else
                    images.append(image.fullsize);
            }
        }
        return images;
    } else {
        return QString();
    }
}

}
}

#endif // LEXICONS_FUNC_UNKNOWN_CPP
