#include "appbskycontactsendnotification.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyContactSendNotification::AppBskyContactSendNotification(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyContactSendNotification::sendNotification(const QString &from, const QString &to)
{
    QJsonObject json_obj;
    if (!from.isEmpty()) {
        json_obj.insert(QStringLiteral("from"), from);
    }
    if (!to.isEmpty()) {
        json_obj.insert(QStringLiteral("to"), to);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.contact.sendNotification"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyContactSendNotification::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
