#include "appbskygraphgetlistswithmembership.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetListsWithMembership::AppBskyGraphGetListsWithMembership(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyGraphGetListsWithMembership::getListsWithMembership(const QString &actor,
                                                                const int limit,
                                                                const QString &cursor,
                                                                const QList<QString> &purposes)
{
    QUrlQuery url_query;
    if (!actor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("actor"), actor);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }
    for (const auto &value : purposes) {
        url_query.addQueryItem(QStringLiteral("purposes"), value);
    }

    get(QStringLiteral("xrpc/app.bsky.graph.getListsWithMembership"), url_query);
}

const QList<AtProtocolType::AppBskyGraphGetListsWithMembership::ListWithMembership> &
AppBskyGraphGetListsWithMembership::listsWithMembershipList() const
{
    return m_listsWithMembershipList;
}

bool AppBskyGraphGetListsWithMembership::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("listsWithMembership")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &value : json_doc.object().value("listsWithMembership").toArray()) {
            AtProtocolType::AppBskyGraphGetListsWithMembership::ListWithMembership data;
            AtProtocolType::AppBskyGraphGetListsWithMembership::copyListWithMembership(
                    value.toObject(), data);
            m_listsWithMembershipList.append(data);
        }
    }

    return success;
}

}
