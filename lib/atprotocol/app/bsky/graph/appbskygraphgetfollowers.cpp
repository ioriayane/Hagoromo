#include "appbskygraphgetfollowers.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetFollowers::AppBskyGraphGetFollowers(QObject *parent)
    : AppBskyGraphGetFollows { parent }
{
    m_listKey = QStringLiteral("followers");
}

void AppBskyGraphGetFollowers::getFollowers(const QString &actor, const int limit,
                                            const QString &cursor, const QString &sort)
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
    if (!sort.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("sort"), sort);
    }

    get(QStringLiteral("xrpc/app.bsky.graph.getFollowers"), url_query);
}

}
