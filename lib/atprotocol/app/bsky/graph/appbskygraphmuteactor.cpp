#include "appbskygraphmuteactor.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyGraphMuteActor::AppBskyGraphMuteActor(QObject *parent) : AccessAtProtocol { parent } { }

void AppBskyGraphMuteActor::muteActor(const QString &actor)
{
    QJsonObject json_obj;
    json_obj.insert("actor", actor);
    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.graph.muteActor"), json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyGraphMuteActor::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)

    return success;
}

}
