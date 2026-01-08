#include "appbskyfeedgetactorlikes.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetActorLikes::AppBskyFeedGetActorLikes(QObject *parent)
    : AppBskyFeedGetTimeline { parent }
{
    m_listKey = QStringLiteral("feed");
}

void AppBskyFeedGetActorLikes::getActorLikes(const QString &actor, const qint64 limit,
                                             const QString &cursor)
{
    QUrlQuery url_query;
    if (!actor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("actor"), actor);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getActorLikes"), url_query);
}

}
