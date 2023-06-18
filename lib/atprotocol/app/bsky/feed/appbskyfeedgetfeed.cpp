#include "appbskyfeedgetfeed.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

using namespace AtProtocolType;

namespace AtProtocolInterface {

AppBskyFeedGetFeed::AppBskyFeedGetFeed(QObject *parent) : AppBskyFeedGetTimeline { parent } { }

void AppBskyFeedGetFeed::getFeed(const QString &feed, const int limit, const QString &cursor)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("feed"), feed);
    query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getFeed"), query);
}

}
