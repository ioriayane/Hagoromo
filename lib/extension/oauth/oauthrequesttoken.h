#ifndef OAUTHREQUESTTOKEN_H
#define OAUTHREQUESTTOKEN_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class OauthRequestToken : public AccessAtProtocol
{
public:
    explicit OauthRequestToken(QObject *parent = nullptr);

    void requestToken(const QByteArray &payload);

    const AtProtocolType::OauthDefs::TokenResponse &tokenResponse() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::OauthDefs::TokenResponse m_tokenResponse;
};

}

#endif // OAUTHREQUESTTOKEN_H
