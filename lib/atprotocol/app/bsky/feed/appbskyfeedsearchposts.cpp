#include "appbskyfeedsearchposts.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedSearchPosts::AppBskyFeedSearchPosts(QObject *parent) : AppBskyFeedGetPosts { parent }
{
    m_listKey = QStringLiteral("posts");
}

void AppBskyFeedSearchPosts::searchPosts(const QString &q, const int limit, const QString &cursor)
{
    QUrlQuery url_query;
    if (!q.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("q"), q);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.searchPosts"), url_query);
}

}
