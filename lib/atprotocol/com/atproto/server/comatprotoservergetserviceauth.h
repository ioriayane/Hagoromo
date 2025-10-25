#ifndef COMATPROTOSERVERGETSERVICEAUTH_H
#define COMATPROTOSERVERGETSERVICEAUTH_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoServerGetServiceAuth : public AccessAtProtocol
{
public:
    explicit ComAtprotoServerGetServiceAuth(QObject *parent = nullptr);

    void getServiceAuth(const QString &aud, const qint64 exp, const QString &lxm);

    const QString &token() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QString m_token;
};

}

#endif // COMATPROTOSERVERGETSERVICEAUTH_H
