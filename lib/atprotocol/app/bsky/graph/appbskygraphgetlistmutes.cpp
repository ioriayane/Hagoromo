#include "appbskygraphgetlistmutes.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetListMutes::AppBskyGraphGetListMutes(QObject *parent)
    : AppBskyGraphGetLists { parent }
{
}

void AppBskyGraphGetListMutes::getListMutes(const int limit, const QString &cursor)
{
    QUrlQuery query;
    if (limit > 0) {
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.graph.getListMutes"), query);
}

}
