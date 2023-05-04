#include "appbskyfeedgettimeline.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

AppBskyFeedGetTimeline::AppBskyFeedGetTimeline(QObject *parent) : AccessAtProtocol { parent } { }

void AppBskyFeedGetTimeline::getTimeline()
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("actor"), handle());
    //    query.addQueryItem(QStringLiteral("actor"), cursor);

    get(QStringLiteral("xrpc/app.bsky.feed.getTimeline"), query);
}

const QList<AppBskyFeedDefs::FeedViewPost> *AppBskyFeedGetTimeline::feedList() const
{
    return &m_feedList;
}

void AppBskyFeedGetTimeline::parseJson(const QString reply_json)
{
    bool success = false;
    m_feedList.clear();

    //    qDebug() << reply_json;

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        qDebug() << "EMPTY";
        //    } else if (!json_doc.isArray()) {
        //        qDebug() << "NOT ARRAY";
    } else if (!json_doc.object().contains("feed")) {
        qDebug() << "Not found feed";
    } else {
        for (const auto &obj : json_doc.object().value("feed").toArray()) {
            AppBskyFeedDefs::FeedViewPost feed_item;

            QJsonObject json_post = obj.toObject().value("post").toObject();
            feed_item.post.cid = json_post.value("cid").toString();
            feed_item.post.author.displayName =
                    json_post.value("author").toObject().value("displayName").toString();
            feed_item.post.indexedAt = json_post.value("indexedAt").toString();
            feed_item.post.repostCount = json_post.value("repostCount").toInt();
            feed_item.post.repostCount = json_post.value("repostCount").toInt();
            feed_item.post.likeCount = json_post.value("likeCount").toInt();
            copyAuthor(json_post.value("author").toObject(), feed_item.post.author);
            QJsonObject json_record = json_post.value("record").toObject();
            if (!json_record.isEmpty()) {
                if (json_record.value("$type").toString() == QStringLiteral("app.bsky.feed.post")) {
                    AppBskyFeedPost::Record record;
                    record.text = json_record.value("text").toString();
                    record.createdAt = json_record.value("createdAt").toString();
                    feed_item.post.record.setValue<AppBskyFeedPost::Record>(record);
                }
            }
            QJsonObject json_embed = json_post.value("embed").toObject();
            if (!json_embed.isEmpty()) {
                if (json_embed.value("$type").toString()
                    == QStringLiteral("app.bsky.embed.images#view")) {
                    feed_item.post.embed_type =
                            AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedImages_View;
                    for (const auto &json_image : json_embed.value("images").toArray()) {
                        AppBskyEmbedImages::ViewImage image;
                        image.thumb = json_image.toObject().value("thumb").toString();
                        image.fullsize = json_image.toObject().value("fullsize").toString();
                        image.alt = json_image.toObject().value("alt").toString();
                        feed_item.post.embed_AppBskyEmbedImages_View.images.append(image);
                    }
                }
            }

            QJsonObject json_reply = obj.toObject().value("reply").toObject();
            if (!json_reply.isEmpty()) {
                QJsonObject json_parent = json_reply.value("parent").toObject();
                feed_item.reply.parent.cid = json_parent.value("cid").toString();
                copyAuthor(json_parent.value("author").toObject(), feed_item.reply.parent.author);
            }

            m_feedList.append(feed_item);
        }
    }

    emit finished(success);
}

void AppBskyFeedGetTimeline::copyAuthor(const QJsonObject &json_author,
                                        AppBskyActorDefs::ProfileViewBasic &author)
{
    if (!json_author.isEmpty()) {
        author.avatar = json_author.value("avatar").toString();
        author.did = json_author.value("did").toString();
        author.displayName = json_author.value("displayName").toString();
        author.handle = json_author.value("handle").toString();
        //            feed_item.post.author.labels = author.value("labels");
    }
}
