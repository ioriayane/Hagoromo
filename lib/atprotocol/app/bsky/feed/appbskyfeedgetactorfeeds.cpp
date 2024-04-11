#include "appbskyfeedgetactorfeeds.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetActorFeeds::AppBskyFeedGetActorFeeds(QObject *parent)
    : AppBskyFeedGetFeedGenerators { parent }
{
    m_listKey = QStringLiteral("feeds");
}

void AppBskyFeedGetActorFeeds::getActorFeeds(const QString &actor, const int limit,
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

    get(QStringLiteral("xrpc/app.bsky.feed.getActorFeeds"), url_query);
}

}
