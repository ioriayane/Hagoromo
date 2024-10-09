#include "oauthrequesttoken.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

OauthRequestToken::OauthRequestToken(QObject *parent) : AccessAtProtocol { parent } { }

void OauthRequestToken::requestToken(const QByteArray &payload)
{
    post(QString(""), payload, false);
}

const AtProtocolType::OauthDefs::TokenResponse &OauthRequestToken::tokenResponse() const
{
    return m_tokenResponse;
}

bool OauthRequestToken::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::OauthDefs::copyTokenResponse(json_doc.object(), m_tokenResponse);
    }

    return success;
}

}
