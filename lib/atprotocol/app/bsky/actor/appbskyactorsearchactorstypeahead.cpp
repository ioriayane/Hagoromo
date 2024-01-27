#include "appbskyactorsearchactorstypeahead.h"

#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyActorSearchActorsTypeahead::AppBskyActorSearchActorsTypeahead(QObject *parent)
    : AppBskyGraphGetFollows { parent }
{
    m_listKey = QStringLiteral("actors");
}

void AppBskyActorSearchActorsTypeahead::searchActorsTypeahead(const QString &term, const QString &q,
                                                              const int limit)
{
    // term : DEPRECATED: use 'q' instead.
    QUrlQuery query;
    if (!q.isEmpty()) {
        query.addQueryItem(QStringLiteral("q"), q);
    }
    if (limit > 0) {
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }

    get(QStringLiteral("xrpc/app.bsky.actor.searchActorsTypeahead"), query);
}

// 本当はcopyProfileViewBasicで取得するべきだけど、ProfileViewBasicはProfileViewの
// フィールド少ない版なので、親クラスのparseJsonをそのまま使用する
}
