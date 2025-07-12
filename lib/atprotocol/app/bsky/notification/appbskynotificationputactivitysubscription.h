#ifndef APPBSKYNOTIFICATIONPUTACTIVITYSUBSCRIPTION_H
#define APPBSKYNOTIFICATIONPUTACTIVITYSUBSCRIPTION_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyNotificationPutActivitySubscription : public AccessAtProtocol
{
public:
    explicit AppBskyNotificationPutActivitySubscription(QObject *parent = nullptr);

    void putActivitySubscription(const QString &subject, const QJsonObject &activitySubscription);

    const QString &subject() const;
    const AtProtocolType::AppBskyNotificationDefs::ActivitySubscription &
    activitySubscription() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QString m_subject;
    AtProtocolType::AppBskyNotificationDefs::ActivitySubscription m_activitySubscription;
};

}

#endif // APPBSKYNOTIFICATIONPUTACTIVITYSUBSCRIPTION_H
