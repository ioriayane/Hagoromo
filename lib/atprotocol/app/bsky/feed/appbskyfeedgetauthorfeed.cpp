#include "appbskyfeedgetauthorfeed.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetAuthorFeed::AppBskyFeedGetAuthorFeed(QObject *parent)
    : AppBskyFeedGetTimeline { parent }
{
    m_listKey = QStringLiteral("feed");
}

void AppBskyFeedGetAuthorFeed::getAuthorFeed(const QString &actor, const int limit,
                                             const QString &cursor, const QString &filter)
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
    if (!filter.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("filter"), filter);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getAuthorFeed"), url_query);
}

}
