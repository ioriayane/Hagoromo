#ifndef APPBSKYCONTACTSTARTPHONEVERIFICATION_H
#define APPBSKYCONTACTSTARTPHONEVERIFICATION_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyContactStartPhoneVerification : public AccessAtProtocol
{
public:
    explicit AppBskyContactStartPhoneVerification(QObject *parent = nullptr);

    void startPhoneVerification(const QString &phone);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYCONTACTSTARTPHONEVERIFICATION_H
