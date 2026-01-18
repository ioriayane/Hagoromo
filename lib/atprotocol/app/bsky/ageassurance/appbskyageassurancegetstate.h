#ifndef APPBSKYAGEASSURANCEGETSTATE_H
#define APPBSKYAGEASSURANCEGETSTATE_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyAgeassuranceGetState : public AccessAtProtocol
{
public:
    explicit AppBskyAgeassuranceGetState(QObject *parent = nullptr);

    void getState(const QString &countryCode, const QString &regionCode);

    const AtProtocolType::AppBskyAgeassuranceDefs::State &state() const;
    const AtProtocolType::AppBskyAgeassuranceDefs::StateMetadata &metadata() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyAgeassuranceDefs::State m_state;
    AtProtocolType::AppBskyAgeassuranceDefs::StateMetadata m_metadata;
};

}

#endif // APPBSKYAGEASSURANCEGETSTATE_H
