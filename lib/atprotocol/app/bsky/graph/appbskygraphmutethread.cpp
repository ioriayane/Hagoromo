#include "appbskygraphmutethread.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyGraphMuteThread::AppBskyGraphMuteThread(QObject *parent) : AccessAtProtocol { parent } { }

void AppBskyGraphMuteThread::muteThread(const QString &root)
{
    QJsonObject json_obj;
    if (!root.isEmpty()) {
        json_obj.insert(QStringLiteral("root"), root);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.graph.muteThread"), json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyGraphMuteThread::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
