#ifndef APPBSKYUNSPECCEDINITAGEASSURANCE_H
#define APPBSKYUNSPECCEDINITAGEASSURANCE_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyUnspeccedInitAgeAssurance : public AccessAtProtocol
{
public:
    explicit AppBskyUnspeccedInitAgeAssurance(QObject *parent = nullptr);

    void initAgeAssurance(const QString &email, const QString &language,
                          const QString &countryCode);

    const AtProtocolType::AppBskyUnspeccedDefs::AgeAssuranceState &ageAssuranceState() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyUnspeccedDefs::AgeAssuranceState m_ageAssuranceState;
};

}

#endif // APPBSKYUNSPECCEDINITAGEASSURANCE_H
