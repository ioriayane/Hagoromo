#include "appbskygraphgetlistblocks.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetListBlocks::AppBskyGraphGetListBlocks(QObject *parent)
    : AppBskyGraphGetLists { parent }
{
    m_listKey = QStringLiteral("lists");
}

void AppBskyGraphGetListBlocks::getListBlocks(const int limit, const QString &cursor)
{
    QUrlQuery url_query;
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.graph.getListBlocks"), url_query);
}

}
