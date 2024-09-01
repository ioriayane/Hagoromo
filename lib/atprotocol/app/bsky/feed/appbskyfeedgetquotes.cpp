#include "appbskyfeedgetquotes.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetQuotes::AppBskyFeedGetQuotes(QObject *parent) : AppBskyFeedGetPosts { parent }
{
    m_listKey = QStringLiteral("posts");
}

void AppBskyFeedGetQuotes::getQuotes(const QString &uri, const QString &cid, const int limit,
                                     const QString &cursor)
{
    QUrlQuery url_query;
    if (!uri.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("uri"), uri);
    }
    if (!cid.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cid"), cid);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getQuotes"), url_query);
}

}
