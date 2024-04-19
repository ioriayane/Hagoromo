#include "appbskyactorputpreferences.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyActorPutPreferences::AppBskyActorPutPreferences(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyActorPutPreferences::putPreferences(const QJsonArray &preferences)
{
    QJsonObject json_obj;
    if (!preferences.isEmpty()) {
        json_obj.insert(QStringLiteral("preferences"), preferences);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.actor.putPreferences"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyActorPutPreferences::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
