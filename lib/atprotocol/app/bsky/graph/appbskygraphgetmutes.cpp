#include "appbskygraphgetmutes.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetMutes::AppBskyGraphGetMutes(QObject *parent) : AppBskyGraphGetFollows { parent }
{
    m_listKey = QStringLiteral("mutes");
}

void AppBskyGraphGetMutes::getMutes(const int limit, const QString &cursor)
{
    QUrlQuery url_query;
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.graph.getMutes"), url_query);
}

}
