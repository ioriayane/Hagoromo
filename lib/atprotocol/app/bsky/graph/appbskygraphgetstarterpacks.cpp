#include "appbskygraphgetstarterpacks.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetStarterPacks::AppBskyGraphGetStarterPacks(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyGraphGetStarterPacks::getStarterPacks(const QList<QString> &uris)
{
    QUrlQuery url_query;
    for (const auto &value : uris) {
        url_query.addQueryItem(QStringLiteral("uris"), value);
    }

    get(QStringLiteral("xrpc/app.bsky.graph.getStarterPacks"), url_query);
}

const QList<AtProtocolType::AppBskyGraphDefs::StarterPackViewBasic> &
AppBskyGraphGetStarterPacks::starterPacksList() const
{
    return m_starterPacksList;
}

bool AppBskyGraphGetStarterPacks::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("starterPacks")) {
        success = false;
    } else {
        for (const auto &value : json_doc.object().value("starterPacks").toArray()) {
            AtProtocolType::AppBskyGraphDefs::StarterPackViewBasic data;
            AtProtocolType::AppBskyGraphDefs::copyStarterPackViewBasic(value.toObject(), data);
            m_starterPacksList.append(data);
        }
    }

    return success;
}

}
