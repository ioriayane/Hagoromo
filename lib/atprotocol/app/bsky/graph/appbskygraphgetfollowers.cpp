#include "appbskygraphgetfollowers.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetFollowers::AppBskyGraphGetFollowers(QObject *parent)
    : AppBskyGraphGetFollows { parent }
{
    m_listKey = QStringLiteral("followers");
}

bool AppBskyGraphGetFollowers::getFollowers(const QString &actor, const int limit,
                                            const QString &cursor)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("actor"), actor);
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    return get(QStringLiteral("xrpc/app.bsky.graph.getFollowers"), query);
}

}
