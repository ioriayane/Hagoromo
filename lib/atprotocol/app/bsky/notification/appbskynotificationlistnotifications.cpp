#include "appbskynotificationlistnotifications.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyNotificationListNotifications::AppBskyNotificationListNotifications(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyNotificationListNotifications::listNotifications(const QString &cursor)
{
    QUrlQuery query;
    //    query.addQueryItem(QStringLiteral("actor"), handle());
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.notification.listNotifications"), query);
}

const QList<AtProtocolType::AppBskyNotificationListNotifications::Notification> *
AppBskyNotificationListNotifications::notificationList() const
{
    return &m_notificationList;
}

void AppBskyNotificationListNotifications::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("notifications")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &obj : json_doc.object().value("notifications").toArray()) {
            AtProtocolType::AppBskyNotificationListNotifications::Notification notification;

            AtProtocolType::AppBskyNotificationListNotifications::copyNotification(obj.toObject(),
                                                                                   notification);
            m_notificationList.append(notification);
        }
    }

    emit finished(success);
}

}
