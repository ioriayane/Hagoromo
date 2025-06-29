#include "appbskygraphgetfollowers.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetFollowers::AppBskyGraphGetFollowers(QObject *parent)
    : AppBskyGraphGetFollows { parent }
{
    m_listKey = QStringLiteral("followers");
}

void AppBskyGraphGetFollowers::getFollowers(const QString &actor, const qint64 limit,
                                            const QString &cursor)
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

    get(QStringLiteral("xrpc/app.bsky.graph.getFollowers"), url_query);
}

}
