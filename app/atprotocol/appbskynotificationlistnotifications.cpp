#include "appbskynotificationlistnotifications.h"
#include "../atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyNotificationListNotifications::AppBskyNotificationListNotifications(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyNotificationListNotifications::listNotifications()
{
    QUrlQuery query;
    //    query.addQueryItem(QStringLiteral("actor"), handle());
    //    query.addQueryItem(QStringLiteral("cursor"), cursor);

    get(QStringLiteral("xrpc/app.bsky.notification.listNotifications"), query);
}

void AppBskyNotificationListNotifications::parseJson(const QString reply_json)
{
    bool success = false;

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        qDebug() << "EMPTY";
    } else if (!json_doc.object().contains("notifications")) {
        qDebug() << "Not found 'notifications'.";
    } else {
        for (const auto &obj : json_doc.object().value("notifications").toArray()) {
            //            AppBskyFeedDefs::FeedViewPost feed_item;

            //            AppBskyFeedDefs::copyFeedViewPost(obj.toObject(), feed_item);

            //            m_feedList.append(feed_item);
        }
    }

    emit finished(success);
}

}
