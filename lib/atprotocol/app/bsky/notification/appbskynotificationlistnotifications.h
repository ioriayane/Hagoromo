#ifndef APPBSKYNOTIFICATIONLISTNOTIFICATIONS_H
#define APPBSKYNOTIFICATIONLISTNOTIFICATIONS_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyNotificationListNotifications : public AccessAtProtocol
{
public:
    explicit AppBskyNotificationListNotifications(QObject *parent = nullptr);

    void listNotifications(const QString &cursor);

    const QList<AtProtocolType::AppBskyNotificationListNotifications::Notification> *
    notificationList() const;

private:
    virtual void parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyNotificationListNotifications::Notification> m_notificationList;
};

}

#endif // APPBSKYNOTIFICATIONLISTNOTIFICATIONS_H
