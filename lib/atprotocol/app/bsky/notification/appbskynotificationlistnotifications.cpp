#include "appbskynotificationlistnotifications.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyNotificationListNotifications::AppBskyNotificationListNotifications(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyNotificationListNotifications::listNotifications(const int limit, const QString &cursor,
                                                             const QString &seenAt)
{
    QUrlQuery url_query;
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }
    if (!seenAt.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("seenAt"), seenAt);
    }

    get(QStringLiteral("xrpc/app.bsky.notification.listNotifications"), url_query);
}

const QList<AtProtocolType::AppBskyNotificationListNotifications::Notification> &
AppBskyNotificationListNotifications::notificationList() const
{
    return m_notificationList;
}

const QString &AppBskyNotificationListNotifications::seenAt() const
{
    return m_seenAt;
}

bool AppBskyNotificationListNotifications::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("notifications")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &value : json_doc.object().value("notifications").toArray()) {
            AtProtocolType::AppBskyNotificationListNotifications::Notification data;
            AtProtocolType::AppBskyNotificationListNotifications::copyNotification(value.toObject(),
                                                                                   data);
            m_notificationList.append(data);
        }
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("seenAt"),
                                                        m_seenAt);
    }

    return success;
}

}
