#ifndef WELLKNOWNOAUTHAUTHORIZATIONSERVER_H
#define WELLKNOWNOAUTHAUTHORIZATIONSERVER_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class WellKnownOauthAuthorizationServer : public AccessAtProtocol
{
public:
    explicit WellKnownOauthAuthorizationServer(QObject *parent = nullptr);

    void server();

    const AtProtocolType::WellKnownOauthAuthorizationServerDefs::ServerMetadata &
    serverMetadata() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::WellKnownOauthAuthorizationServerDefs::ServerMetadata m_serverMetadata;
};

}

#endif // WELLKNOWNOAUTHAUTHORIZATIONSERVER_H
