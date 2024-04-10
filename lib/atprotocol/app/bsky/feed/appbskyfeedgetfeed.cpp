#include "appbskyfeedgetfeed.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetFeed::AppBskyFeedGetFeed(QObject *parent) : AppBskyFeedGetTimeline { parent }
{
    m_listKey = QStringLiteral("feed");
}

void AppBskyFeedGetFeed::getFeed(const QString &feed, const int limit, const QString &cursor)
{
    QUrlQuery query;
    if (!feed.isEmpty()) {
        query.addQueryItem(QStringLiteral("feed"), feed);
    }
    if (limit > 0) {
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getFeed"), query);
}

}
