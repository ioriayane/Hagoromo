#include "appbskygraphgetstarterpack.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetStarterPack::AppBskyGraphGetStarterPack(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyGraphGetStarterPack::getStarterPack(const QString &starterPack)
{
    QUrlQuery url_query;
    if (!starterPack.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("starterPack"), starterPack);
    }

    get(QStringLiteral("xrpc/app.bsky.graph.getStarterPack"), url_query);
}

const AtProtocolType::AppBskyGraphDefs::StarterPackView &
AppBskyGraphGetStarterPack::starterPack() const
{
    return m_starterPack;
}

bool AppBskyGraphGetStarterPack::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::AppBskyGraphDefs::copyStarterPackView(
                json_doc.object().value("starterPack").toObject(), m_starterPack);
    }

    return success;
}

}
