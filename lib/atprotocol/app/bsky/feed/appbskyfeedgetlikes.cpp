#include "appbskyfeedgetlikes.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetLikes::AppBskyFeedGetLikes(QObject *parent) : AccessAtProtocol { parent } { }

const QList<AtProtocolType::AppBskyFeedGetLikes::Like> *AppBskyFeedGetLikes::likes() const
{
    return &m_likes;
}

void AppBskyFeedGetLikes::getLikes(const QString &uri, const QString &cid, const int limit,
                                   const QString &cursor)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("uri"), uri);
    if (limit > 0) {
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getLikes"), query);
}

bool AppBskyFeedGetLikes::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &obj : json_doc.object().value("likes").toArray()) {
            AtProtocolType::AppBskyFeedGetLikes::Like like;
            AtProtocolType::AppBskyFeedGetLikes::copyLike(obj.toObject(), like);
            m_likes.append(like);
        }
    }

    return success;
}

}
