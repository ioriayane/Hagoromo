#ifndef APPBSKYNOTIFICATIONLISTNOTIFICATIONS_H
#define APPBSKYNOTIFICATIONLISTNOTIFICATIONS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyNotificationListNotifications : public AccessAtProtocol
{
public:
    explicit AppBskyNotificationListNotifications(QObject *parent = nullptr);

    void listNotifications(const QList<QString> &reasons, const int limit, const bool priority,
                           const QString &cursor, const QString &seenAt);

    const QList<AtProtocolType::AppBskyNotificationListNotifications::Notification> &
    notificationsList() const;
    const bool &priority() const;
    const QString &seenAt() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyNotificationListNotifications::Notification> m_notificationsList;
    bool m_priority;
    QString m_seenAt;
};

}

#endif // APPBSKYNOTIFICATIONLISTNOTIFICATIONS_H
