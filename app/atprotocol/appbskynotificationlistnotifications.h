#ifndef APPBSKYNOTIFICATIONLISTNOTIFICATIONS_H
#define APPBSKYNOTIFICATIONLISTNOTIFICATIONS_H

#include "accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyNotificationListNotifications : public AccessAtProtocol
{
public:
    explicit AppBskyNotificationListNotifications(QObject *parent = nullptr);

    void listNotifications();

private:
    virtual void parseJson(const QString reply_json);
};

}

#endif // APPBSKYNOTIFICATIONLISTNOTIFICATIONS_H
