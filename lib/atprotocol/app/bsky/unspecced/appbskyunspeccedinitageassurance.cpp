#include "appbskyunspeccedinitageassurance.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyUnspeccedInitAgeAssurance::AppBskyUnspeccedInitAgeAssurance(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyUnspeccedInitAgeAssurance::initAgeAssurance(const QString &email,
                                                        const QString &language,
                                                        const QString &countryCode)
{
    QJsonObject json_obj;
    if (!email.isEmpty()) {
        json_obj.insert(QStringLiteral("email"), email);
    }
    if (!language.isEmpty()) {
        json_obj.insert(QStringLiteral("language"), language);
    }
    if (!countryCode.isEmpty()) {
        json_obj.insert(QStringLiteral("countryCode"), countryCode);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.unspecced.initAgeAssurance"),
         json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::AppBskyUnspeccedDefs::AgeAssuranceState &
AppBskyUnspeccedInitAgeAssurance::ageAssuranceState() const
{
    return m_ageAssuranceState;
}

bool AppBskyUnspeccedInitAgeAssurance::parseJson(bool success, const QString reply_json)
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
