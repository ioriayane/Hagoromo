#include "appbskyactorsearchactorstypeahead.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyActorSearchActorsTypeahead::AppBskyActorSearchActorsTypeahead(QObject *parent)
    : AppBskyGraphGetFollows { parent }
{
    m_listKey = QStringLiteral("actors");
}

void AppBskyActorSearchActorsTypeahead::searchActorsTypeahead(const QString &q,
                                                              const QString &viewer,
                                                              const int limit)
{
    QUrlQuery url_query;
    if (!q.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("q"), q);
    }
    if (!viewer.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("viewer"), viewer);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }

    get(QStringLiteral("xrpc/app.bsky.actor.searchActorsTypeahead"), url_query);
}

}
