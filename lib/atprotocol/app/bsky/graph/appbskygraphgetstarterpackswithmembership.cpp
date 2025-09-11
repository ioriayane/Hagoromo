#include "appbskygraphgetstarterpackswithmembership.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetStarterPacksWithMembership::AppBskyGraphGetStarterPacksWithMembership(
        QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyGraphGetStarterPacksWithMembership::getStarterPacksWithMembership(const QString &actor,
                                                                              const int limit,
                                                                              const QString &cursor)
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

    get(QStringLiteral("xrpc/app.bsky.graph.getStarterPacksWithMembership"), url_query);
}

const QList<AtProtocolType::AppBskyGraphGetStarterPacksWithMembership::StarterPackWithMembership> &
AppBskyGraphGetStarterPacksWithMembership::starterPacksWithMembershipList() const
{
    return m_starterPacksWithMembershipList;
}

bool AppBskyGraphGetStarterPacksWithMembership::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("starterPacksWithMembership")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &value : json_doc.object().value("starterPacksWithMembership").toArray()) {
            AtProtocolType::AppBskyGraphGetStarterPacksWithMembership::StarterPackWithMembership
                    data;
            AtProtocolType::AppBskyGraphGetStarterPacksWithMembership::
                    copyStarterPackWithMembership(value.toObject(), data);
            m_starterPacksWithMembershipList.append(data);
        }
    }

    return success;
}

}
