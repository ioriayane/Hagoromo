#include "appbskyageassurancegetstate.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyAgeassuranceGetState::AppBskyAgeassuranceGetState(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyAgeassuranceGetState::getState(const QString &countryCode, const QString &regionCode)
{
    QUrlQuery url_query;
    if (!countryCode.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("countryCode"), countryCode);
    }
    if (!regionCode.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("regionCode"), regionCode);
    }

    get(QStringLiteral("xrpc/app.bsky.ageassurance.getState"), url_query);
}

const AtProtocolType::AppBskyAgeassuranceDefs::State &AppBskyAgeassuranceGetState::state() const
{
    return m_state;
}

const AtProtocolType::AppBskyAgeassuranceDefs::StateMetadata &
AppBskyAgeassuranceGetState::metadata() const
{
    return m_metadata;
}

bool AppBskyAgeassuranceGetState::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::AppBskyAgeassuranceDefs::copyState(
                json_doc.object().value("state").toObject(), m_state);
        AtProtocolType::AppBskyAgeassuranceDefs::copyStateMetadata(
                json_doc.object().value("metadata").toObject(), m_metadata);
    }

    return success;
}

}
