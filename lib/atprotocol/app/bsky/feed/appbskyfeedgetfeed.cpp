#include "appbskyfeedgetfeed.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetFeed::AppBskyFeedGetFeed(QObject *parent) : AppBskyFeedGetTimeline { parent }
{
    m_listKey = QStringLiteral("feed");
}

void AppBskyFeedGetFeed::getFeed(const QString &feed, const int limit, const QString &cursor)
{
    QUrlQuery url_query;
    if (!feed.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("feed"), feed);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getFeed"), url_query);
}

}
