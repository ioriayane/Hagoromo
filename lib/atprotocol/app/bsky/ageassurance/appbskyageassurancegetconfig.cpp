#include "appbskyageassurancegetconfig.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyAgeassuranceGetConfig::AppBskyAgeassuranceGetConfig(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyAgeassuranceGetConfig::getConfig()
{
    QUrlQuery url_query;

    get(QStringLiteral("xrpc/app.bsky.ageassurance.getConfig"), url_query);
}

const AtProtocolType::AppBskyAgeassuranceDefs::Config &AppBskyAgeassuranceGetConfig::config() const
{
    return m_config;
}

bool AppBskyAgeassuranceGetConfig::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::AppBskyAgeassuranceDefs::copyConfig(json_doc.object(), m_config);
    }

    return success;
}

}
