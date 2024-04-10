#include "appbskygraphgetfollowers.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetFollowers::AppBskyGraphGetFollowers(QObject *parent)
    : AppBskyGraphGetFollows { parent }
{
    m_listKey = QStringLiteral("followers");
}

void AppBskyGraphGetFollowers::getFollowers(const QString &actor, const int limit,
                                            const QString &cursor)
{
    QUrlQuery query;
    if (!actor.isEmpty()) {
        query.addQueryItem(QStringLiteral("actor"), actor);
    }
    if (limit > 0) {
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.graph.getFollowers"), query);
}

}
