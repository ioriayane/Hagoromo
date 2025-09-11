#ifndef APPBSKYUNSPECCEDGETAGEASSURANCESTATE_H
#define APPBSKYUNSPECCEDGETAGEASSURANCESTATE_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyUnspeccedGetAgeAssuranceState : public AccessAtProtocol
{
public:
    explicit AppBskyUnspeccedGetAgeAssuranceState(QObject *parent = nullptr);

    void getAgeAssuranceState();

    const AtProtocolType::AppBskyUnspeccedDefs::AgeAssuranceState &ageAssuranceState() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyUnspeccedDefs::AgeAssuranceState m_ageAssuranceState;
};

}

#endif // APPBSKYUNSPECCEDGETAGEASSURANCESTATE_H
