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
        // typeに#以降がないのでmainの定義で参照
        AppBskyFeedPost::Record record;
        record.text = src.value("text").toString();
        record.createdAt = src.value("createdAt").toString();
        dest.setValue<AppBskyFeedPost::Record>(record);

    } else if (type == QStringLiteral("app.bsky.feed.like")) {
        AppBskyFeedLike::Record record;
        record.subject.cid = src.value("subject").toObject().value("cid").toString();
        record.subject.uri = src.value("subject").toObject().value("uri").toString();
        record.createdAt = src.value("createdAt").toString();
        dest.setValue<AppBskyFeedLike::Record>(record);

    } else if (type == QStringLiteral("app.bsky.feed.repost")) {
        AppBskyFeedRepost::Record record;
        ComAtprotoRepoStrongRef::copyMain(src.value("subject").toObject(), record.subject);
        record.createdAt = src.value("createdAt").toString();
        dest.setValue<AppBskyFeedRepost::Record>(record);
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
