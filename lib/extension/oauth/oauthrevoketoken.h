#ifndef OAUTHREVOKETOKEN_H
#define OAUTHREVOKETOKEN_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

// トークンの失効(RFC 7009)
class OauthRevokeToken : public AccessAtProtocol
{
public:
    explicit OauthRevokeToken(QObject *parent = nullptr);

    void revokeToken(const QByteArray &payload);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // OAUTHREVOKETOKEN_H
