#include "appbskynotificationputpreferences.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyNotificationPutPreferences::AppBskyNotificationPutPreferences(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyNotificationPutPreferences::putPreferences(const bool priority)
{
    QJsonObject json_obj;
    json_obj.insert(QStringLiteral("priority"), priority);

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.notification.putPreferences"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyNotificationPutPreferences::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
