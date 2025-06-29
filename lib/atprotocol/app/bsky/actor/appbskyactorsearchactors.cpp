#include "appbskyactorsearchactors.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyActorSearchActors::AppBskyActorSearchActors(QObject *parent)
    : AppBskyGraphGetFollows { parent }
{
    m_listKey = QStringLiteral("actors");
}

void AppBskyActorSearchActors::searchActors(const QString &q, const qint64 limit,
                                            const QString &cursor)
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

    get(QStringLiteral("xrpc/app.bsky.actor.searchActors"), url_query);
}

}
