#ifndef COMATPROTOIDENTITYRESOLVEHANDLE_H
#define COMATPROTOIDENTITYRESOLVEHANDLE_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoIdentityResolveHandle : public AccessAtProtocol
{
public:
    explicit ComAtprotoIdentityResolveHandle(QObject *parent = nullptr);

    void resolveHandle(const QString &handle);

    QString did() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QString m_did;
};

}

#endif // COMATPROTOIDENTITYRESOLVEHANDLE_H
