#include "appbskyfeedgetactorlikes.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

using namespace AtProtocolType;

namespace AtProtocolInterface {

AppBskyFeedGetActorLikes::AppBskyFeedGetActorLikes(QObject *parent)
    : AppBskyFeedGetTimeline { parent }
{
}

void AppBskyFeedGetActorLikes::getActorLikes(const QString &actor, const int limit,
                                             const QString &cursor)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("actor"), actor);
    if (limit > 0) {
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getActorLikes"), query);
}

}
