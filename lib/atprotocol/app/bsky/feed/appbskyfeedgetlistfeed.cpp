#include "appbskyfeedgetlistfeed.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

using namespace AtProtocolType;

namespace AtProtocolInterface {

AppBskyFeedGetListFeed::AppBskyFeedGetListFeed(QObject *parent) : AppBskyFeedGetTimeline { parent }
{
}

void AppBskyFeedGetListFeed::getListFeed(const QString &list, const int limit,
                                         const QString &cursor)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("list"), list);
    if (limit > 0) {
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getListFeed"), query);
}

}
