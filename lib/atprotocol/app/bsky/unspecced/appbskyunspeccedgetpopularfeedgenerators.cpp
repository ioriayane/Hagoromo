#include "appbskyunspeccedgetpopularfeedgenerators.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyUnspeccedGetPopularFeedGenerators::AppBskyUnspeccedGetPopularFeedGenerators(QObject *parent)
    : AppBskyFeedGetFeedGenerators { parent }
{
    m_listKey = QStringLiteral("feeds");
}

void AppBskyUnspeccedGetPopularFeedGenerators::getPopularFeedGenerators(const qint64 limit,
                                                                        const QString &cursor,
                                                                        const QString &query)
{
    QUrlQuery url_query;
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }
    if (!query.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("query"), query);
    }

    get(QStringLiteral("xrpc/app.bsky.unspecced.getPopularFeedGenerators"), url_query);
}

}
