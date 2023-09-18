#include "appbskygraphunmuteactor.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyGraphUnmuteActor::AppBskyGraphUnmuteActor(QObject *parent) : AccessAtProtocol { parent } { }

void AppBskyGraphUnmuteActor::unmuteActor(const QString &actor)
{
    QJsonObject json_obj;
    json_obj.insert("actor", actor);
    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.graph.unmuteActor"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyGraphUnmuteActor::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)

    return success;
}

}
