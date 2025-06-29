#include "appbskyfeedgetlistfeed.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetListFeed::AppBskyFeedGetListFeed(QObject *parent) : AppBskyFeedGetTimeline { parent }
{
    m_listKey = QStringLiteral("feed");
}

void AppBskyFeedGetListFeed::getListFeed(const QString &list, const qint64 limit,
                                         const QString &cursor)
{
    QUrlQuery url_query;
    if (!list.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("list"), list);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getListFeed"), url_query);
}

}
