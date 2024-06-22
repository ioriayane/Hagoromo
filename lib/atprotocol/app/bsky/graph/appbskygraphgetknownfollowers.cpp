#include "appbskygraphgetknownfollowers.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetKnownFollowers::AppBskyGraphGetKnownFollowers(QObject *parent)
    : AppBskyGraphGetFollows { parent }
{
    m_listKey = QStringLiteral("followers");
}

void AppBskyGraphGetKnownFollowers::getKnownFollowers(const QString &actor, const int limit,
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

    get(QStringLiteral("xrpc/app.bsky.graph.getKnownFollowers"), url_query);
}

}
