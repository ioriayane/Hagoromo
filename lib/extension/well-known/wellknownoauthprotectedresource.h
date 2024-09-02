#ifndef WELLKNOWNOAUTHPROTECTEDRESOURCE_H
#define WELLKNOWNOAUTHPROTECTEDRESOURCE_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class WellKnownOauthProtectedResource : public AccessAtProtocol
{
public:
    explicit WellKnownOauthProtectedResource(QObject *parent = nullptr);

    void resource();

    const AtProtocolType::WellKnownOauthProtectedResource::OAuthProtectedResource &
    OAuthProtectedResource() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::WellKnownOauthProtectedResource::OAuthProtectedResource
            m_OAuthProtectedResource;
};

}

#endif // WELLKNOWNOAUTHPROTECTEDRESOURCE_H
