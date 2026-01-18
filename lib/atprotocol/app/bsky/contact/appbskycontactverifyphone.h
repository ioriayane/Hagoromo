#ifndef APPBSKYCONTACTVERIFYPHONE_H
#define APPBSKYCONTACTVERIFYPHONE_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyContactVerifyPhone : public AccessAtProtocol
{
public:
    explicit AppBskyContactVerifyPhone(QObject *parent = nullptr);

    void verifyPhone(const QString &phone, const QString &code);

    const QString &token() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QString m_token;
};

}

#endif // APPBSKYCONTACTVERIFYPHONE_H
