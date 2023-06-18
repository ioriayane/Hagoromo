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

void AppBskyActorPutPreferences::parseJson(const QString reply_json)
{
    bool success = false;

    //    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    //    if (json_doc.isEmpty()) {
    //    } else {
    //    }
    success = true;

    emit finished(success);
}

}
