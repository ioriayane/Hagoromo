#ifndef APPBSKYCONTACTSENDNOTIFICATION_H
#define APPBSKYCONTACTSENDNOTIFICATION_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyContactSendNotification : public AccessAtProtocol
{
public:
    explicit AppBskyContactSendNotification(QObject *parent = nullptr);

    void sendNotification(const QString &from, const QString &to);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYCONTACTSENDNOTIFICATION_H
