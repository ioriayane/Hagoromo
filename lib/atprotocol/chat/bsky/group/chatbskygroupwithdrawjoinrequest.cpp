#include "chatbskygroupwithdrawjoinrequest.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyGroupWithdrawJoinRequest::ChatBskyGroupWithdrawJoinRequest(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyGroupWithdrawJoinRequest::withdrawJoinRequest(const QString &convoId)
{
    QJsonObject json_obj;
    if (!convoId.isEmpty()) {
        json_obj.insert(QStringLiteral("convoId"), convoId);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.group.withdrawJoinRequest"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool ChatBskyGroupWithdrawJoinRequest::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
