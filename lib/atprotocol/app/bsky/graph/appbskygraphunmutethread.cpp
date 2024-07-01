#include "appbskygraphunmutethread.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyGraphUnmuteThread::AppBskyGraphUnmuteThread(QObject *parent)
    : AccessAtProtocol { parent } { }

void AppBskyGraphUnmuteThread::unmuteThread(const QString &root)
{
    QJsonObject json_obj;
    if (!root.isEmpty()) {
        json_obj.insert(QStringLiteral("root"), root);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.graph.unmuteThread"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyGraphUnmuteThread::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
