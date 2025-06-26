#include "appbskynotificationputactivitysubscription.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyNotificationPutActivitySubscription::AppBskyNotificationPutActivitySubscription(
        QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyNotificationPutActivitySubscription::putActivitySubscription(
        const QString &subject, const QJsonObject &activitySubscription)
{
    QJsonObject json_obj;
    if (!subject.isEmpty()) {
        json_obj.insert(QStringLiteral("subject"), subject);
    }
    if (!activitySubscription.isEmpty()) {
        json_obj.insert(QStringLiteral("activitySubscription"), activitySubscription);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.notification.putActivitySubscription"),
         json_doc.toJson(QJsonDocument::Compact));
}

const QString &AppBskyNotificationPutActivitySubscription::subject() const
{
    return m_subject;
}

const AtProtocolType::AppBskyNotificationDefs::ActivitySubscription &
AppBskyNotificationPutActivitySubscription::activitySubscription() const
{
    return m_activitySubscription;
}

bool AppBskyNotificationPutActivitySubscription::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("subject"),
                                                        m_subject);
        AtProtocolType::AppBskyNotificationDefs::copyActivitySubscription(
                json_doc.object().value("activitySubscription").toObject(), m_activitySubscription);
    }

    return success;
}

}
