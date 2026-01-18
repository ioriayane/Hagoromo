#ifndef APPBSKYAGEASSURANCEGETCONFIG_H
#define APPBSKYAGEASSURANCEGETCONFIG_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyAgeassuranceGetConfig : public AccessAtProtocol
{
public:
    explicit AppBskyAgeassuranceGetConfig(QObject *parent = nullptr);

    void getConfig();

    const AtProtocolType::AppBskyAgeassuranceDefs::Config &config() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyAgeassuranceDefs::Config m_config;
};

}

#endif // APPBSKYAGEASSURANCEGETCONFIG_H
