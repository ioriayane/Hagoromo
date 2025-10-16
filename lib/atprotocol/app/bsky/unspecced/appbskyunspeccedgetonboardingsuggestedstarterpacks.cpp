#include "appbskyunspeccedgetonboardingsuggestedstarterpacks.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyUnspeccedGetOnboardingSuggestedStarterPacks::
        AppBskyUnspeccedGetOnboardingSuggestedStarterPacks(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyUnspeccedGetOnboardingSuggestedStarterPacks::getOnboardingSuggestedStarterPacks(
        const int limit)
{
    QUrlQuery url_query;
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }

    get(QStringLiteral("xrpc/app.bsky.unspecced.getOnboardingSuggestedStarterPacks"), url_query);
}

const QList<AtProtocolType::AppBskyGraphDefs::StarterPackView> &
AppBskyUnspeccedGetOnboardingSuggestedStarterPacks::starterPacksList() const
{
    return m_starterPacksList;
}

bool AppBskyUnspeccedGetOnboardingSuggestedStarterPacks::parseJson(bool success,
                                                                   const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("starterPacks")) {
        success = false;
    } else {
        for (const auto &value : json_doc.object().value("starterPacks").toArray()) {
            AtProtocolType::AppBskyGraphDefs::StarterPackView data;
            AtProtocolType::AppBskyGraphDefs::copyStarterPackView(value.toObject(), data);
            m_starterPacksList.append(data);
        }
    }

    return success;
}

}
