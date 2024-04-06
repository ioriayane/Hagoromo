#include "appbskygraphgetlistblocks.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetListBlocks::AppBskyGraphGetListBlocks(QObject *parent)
    : AppBskyGraphGetLists { parent }
{
}

void AppBskyGraphGetListBlocks::getListBlocks(const int limit, const QString &cursor)
{
    QUrlQuery query;
    if (limit > 0) {
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.graph.getListBlocks"), query);
}

}
