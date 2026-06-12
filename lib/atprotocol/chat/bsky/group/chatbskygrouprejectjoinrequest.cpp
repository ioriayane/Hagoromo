#include "chatbskygrouprejectjoinrequest.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyGroupRejectJoinRequest::ChatBskyGroupRejectJoinRequest(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyGroupRejectJoinRequest::rejectJoinRequest(const QString &convoId,
                                                       const QString &member)
{
    QJsonObject json_obj;
    if (!convoId.isEmpty()) {
        json_obj.insert(QStringLiteral("convoId"), convoId);
    }
    if (!member.isEmpty()) {
        json_obj.insert(QStringLiteral("member"), member);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.group.rejectJoinRequest"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool ChatBskyGroupRejectJoinRequest::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
