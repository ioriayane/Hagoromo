#include "chatbskyactorexportaccountdata.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyActorExportAccountData::ChatBskyActorExportAccountData(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyActorExportAccountData::exportAccountData()
{
    QUrlQuery url_query;

    get(QStringLiteral("xrpc/chat.bsky.actor.exportAccountData"), url_query);
}

bool ChatBskyActorExportAccountData::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
