#include "appbskygraphgetlistmutes.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetListMutes::AppBskyGraphGetListMutes(QObject *parent)
    : AppBskyGraphGetLists { parent }
{
    m_listKey = QStringLiteral("lists");
}

void AppBskyGraphGetListMutes::getListMutes(const qint64 limit, const QString &cursor)
{
    QUrlQuery url_query;
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.graph.getListMutes"), url_query);
}

}
