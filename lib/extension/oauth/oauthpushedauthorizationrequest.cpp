#include "oauthpushedauthorizationrequest.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

OauthPushedAuthorizationRequest::OauthPushedAuthorizationRequest(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void OauthPushedAuthorizationRequest::pushedAuthorizationRequest(const QByteArray &payload)
{
    post(QString(), payload, false);
}

const AtProtocolType::OauthDefs::PushedAuthorizationResponse &
OauthPushedAuthorizationRequest::pushedAuthorizationResponse() const
{
    return m_pushedAuthorizationResponse;
}

bool OauthPushedAuthorizationRequest::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::OauthDefs::copyPushedAuthorizationResponse(json_doc.object(),
                                                                   m_pushedAuthorizationResponse);
    }

    return success;
}

}
