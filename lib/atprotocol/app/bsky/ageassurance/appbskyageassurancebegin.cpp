#include "appbskyageassurancebegin.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyAgeassuranceBegin::AppBskyAgeassuranceBegin(QObject *parent)
    : AccessAtProtocol { parent } { }

void AppBskyAgeassuranceBegin::begin(const QString &email, const QString &language,
                                     const QString &countryCode, const QString &regionCode)
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
    if (!regionCode.isEmpty()) {
        json_obj.insert(QStringLiteral("regionCode"), regionCode);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.ageassurance.begin"),
         json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::AppBskyAgeassuranceDefs::State &AppBskyAgeassuranceBegin::state() const
{
    return m_state;
}

bool AppBskyAgeassuranceBegin::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::AppBskyAgeassuranceDefs::copyState(json_doc.object(), m_state);
    }

    return success;
}

}
