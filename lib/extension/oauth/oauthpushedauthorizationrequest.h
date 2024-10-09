#ifndef OAUTHPUSHEDAUTHORIZATIONREQUEST_H
#define OAUTHPUSHEDAUTHORIZATIONREQUEST_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class OauthPushedAuthorizationRequest : public AccessAtProtocol
{
public:
    explicit OauthPushedAuthorizationRequest(QObject *parent = nullptr);

    void pushedAuthorizationRequest(const QByteArray &payload);

    const AtProtocolType::OauthDefs::PushedAuthorizationResponse &
    pushedAuthorizationResponse() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::OauthDefs::PushedAuthorizationResponse m_pushedAuthorizationResponse;
};

}

#endif // OAUTHPUSHEDAUTHORIZATIONREQUEST_H
