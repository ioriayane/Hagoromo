#include "appbskygraphgetblocks.h"

#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetBlocks::AppBskyGraphGetBlocks(QObject *parent) : AppBskyGraphGetFollows { parent }
{
    m_listKey = QStringLiteral("blocks");
}

void AppBskyGraphGetBlocks::getBlocks(const int limit, const QString &cursor)
{
    QUrlQuery query;
    if (limit > 0) {
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.graph.getBlocks"), query);
}

}
