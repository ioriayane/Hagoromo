#include "appbskygraphgetmutes.h"

#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetMutes::AppBskyGraphGetMutes(QObject *parent) : AppBskyGraphGetFollows { parent }
{
    m_listKey = QStringLiteral("mutes");
}

void AppBskyGraphGetMutes::getMutes(const int limit, const QString &cursor)
{
    QUrlQuery query;
    if (limit > 0) {
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.graph.getMutes"), query);
}

}
