#ifndef APPBSKYAGEASSURANCEBEGIN_H
#define APPBSKYAGEASSURANCEBEGIN_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyAgeassuranceBegin : public AccessAtProtocol
{
public:
    explicit AppBskyAgeassuranceBegin(QObject *parent = nullptr);

    void begin(const QString &email, const QString &language, const QString &countryCode,
               const QString &regionCode);

    const AtProtocolType::AppBskyAgeassuranceDefs::State &state() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyAgeassuranceDefs::State m_state;
};

}

#endif // APPBSKYAGEASSURANCEBEGIN_H
