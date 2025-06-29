#include "appbskygraphgetactorstarterpacks.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetActorStarterPacks::AppBskyGraphGetActorStarterPacks(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyGraphGetActorStarterPacks::getActorStarterPacks(const QString &actor,
                                                            const qint64 limit,
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

    get(QStringLiteral("xrpc/app.bsky.graph.getActorStarterPacks"), url_query);
}

const QList<AtProtocolType::AppBskyGraphDefs::StarterPackViewBasic> &
AppBskyGraphGetActorStarterPacks::starterPacksList() const
{
    return m_starterPacksList;
}

bool AppBskyGraphGetActorStarterPacks::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("starterPacks")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &value : json_doc.object().value("starterPacks").toArray()) {
            AtProtocolType::AppBskyGraphDefs::StarterPackViewBasic data;
            AtProtocolType::AppBskyGraphDefs::copyStarterPackViewBasic(value.toObject(), data);
            m_starterPacksList.append(data);
        }
    }

    return success;
}

}
