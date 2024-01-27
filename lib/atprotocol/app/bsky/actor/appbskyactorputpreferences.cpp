#include "appbskyactorputpreferences.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyActorPutPreferences::AppBskyActorPutPreferences(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyActorPutPreferences::putPreferences(const QString &json)
{
    post(QStringLiteral("xrpc/app.bsky.actor.putPreferences"), json.toUtf8(), true);
}

bool AppBskyActorPutPreferences::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
