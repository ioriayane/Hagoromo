#include "appbskygraphmuteactorlist.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyGraphMuteActorList::AppBskyGraphMuteActorList(QObject *parent) : AccessAtProtocol { parent }
{
}

void AppBskyGraphMuteActorList::muteActorList(const QString &list)
{
    QJsonObject json_obj;
    json_obj.insert("list", list);
    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.graph.muteActorList"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyGraphMuteActorList::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)

    return success;
}

}
