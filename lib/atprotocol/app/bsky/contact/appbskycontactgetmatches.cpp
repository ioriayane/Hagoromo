#include "appbskycontactgetmatches.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyContactGetMatches::AppBskyContactGetMatches(QObject *parent)
    : AppBskyGraphGetFollows { parent }
{
    m_listKey = QStringLiteral("matches");
}

void AppBskyContactGetMatches::getMatches(const int limit, const QString &cursor)
{
    QUrlQuery url_query;
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.contact.getMatches"), url_query);
}

}
