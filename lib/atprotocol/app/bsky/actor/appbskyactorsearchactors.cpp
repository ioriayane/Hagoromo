#include "appbskyactorsearchactors.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyActorSearchActors::AppBskyActorSearchActors(QObject *parent)
    : AppBskyGraphGetFollows { parent }
{
    m_listKey = QStringLiteral("actors");
}

bool AppBskyActorSearchActors::searchActors(const QString &q, const int limit,
                                            const QString &cursor)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("q"), q);
    if (limit > 0) {
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    return get(QStringLiteral("xrpc/app.bsky.actor.searchActors"), query);
}

}
