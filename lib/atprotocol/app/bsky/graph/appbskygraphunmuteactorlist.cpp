#include "appbskygraphunmuteactorlist.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyGraphUnmuteActorList::AppBskyGraphUnmuteActorList(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyGraphUnmuteActorList::unmuteActorList(const QString &list)
{
    QJsonObject json_obj;
    json_obj.insert("list", list);
    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.graph.unmuteActorList"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyGraphUnmuteActorList::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)

    return success;
}

}
