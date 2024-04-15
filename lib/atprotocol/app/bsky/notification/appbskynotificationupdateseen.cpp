#include "appbskynotificationupdateseen.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyNotificationUpdateSeen::AppBskyNotificationUpdateSeen(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyNotificationUpdateSeen::updateSeen(const QString &seenAt)
{
    QJsonObject json_obj;
    if (!seenAt.isEmpty()) {
        json_obj.insert(QStringLiteral("seenAt"), seenAt);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.notification.updateSeen"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyNotificationUpdateSeen::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
