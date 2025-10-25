#include "appbskyunspeccedgetageassurancestate.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyUnspeccedGetAgeAssuranceState::AppBskyUnspeccedGetAgeAssuranceState(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyUnspeccedGetAgeAssuranceState::getAgeAssuranceState()
{
    QUrlQuery url_query;

    get(QStringLiteral("xrpc/app.bsky.unspecced.getAgeAssuranceState"), url_query);
}

const AtProtocolType::AppBskyUnspeccedDefs::AgeAssuranceState &
AppBskyUnspeccedGetAgeAssuranceState::ageAssuranceState() const
{
    return m_ageAssuranceState;
}

bool AppBskyUnspeccedGetAgeAssuranceState::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::AppBskyUnspeccedDefs::copyAgeAssuranceState(json_doc.object(),
                                                                    m_ageAssuranceState);
    }

    return success;
}

}
