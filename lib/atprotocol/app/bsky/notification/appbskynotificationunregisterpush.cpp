#include "appbskynotificationunregisterpush.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyNotificationUnregisterPush::AppBskyNotificationUnregisterPush(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyNotificationUnregisterPush::unregisterPush(const QString &serviceDid,
                                                       const QString &token,
                                                       const QString &platform,
                                                       const QString &appId)
{
    QJsonObject json_obj;
    if (!serviceDid.isEmpty()) {
        json_obj.insert(QStringLiteral("serviceDid"), serviceDid);
    }
    if (!token.isEmpty()) {
        json_obj.insert(QStringLiteral("token"), token);
    }
    if (!platform.isEmpty()) {
        json_obj.insert(QStringLiteral("platform"), platform);
    }
    if (!appId.isEmpty()) {
        json_obj.insert(QStringLiteral("appId"), appId);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.notification.unregisterPush"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyNotificationUnregisterPush::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
