#include "chatbskyactordeleteaccount.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyActorDeleteAccount::ChatBskyActorDeleteAccount(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyActorDeleteAccount::deleteAccount()
{
    post(QStringLiteral("xrpc/chat.bsky.actor.deleteAccount"), QByteArray());
}

bool ChatBskyActorDeleteAccount::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
