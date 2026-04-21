#include "chatbskygroupapprovejoinrequest.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyGroupApproveJoinRequest::ChatBskyGroupApproveJoinRequest(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyGroupApproveJoinRequest::approveJoinRequest(const QString &convoId,
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

    post(QStringLiteral("xrpc/chat.bsky.group.approveJoinRequest"),
         json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::ChatBskyConvoDefs::ConvoView &ChatBskyGroupApproveJoinRequest::convo() const
{
    return m_convo;
}

bool ChatBskyGroupApproveJoinRequest::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::ChatBskyConvoDefs::copyConvoView(
                json_doc.object().value("convo").toObject(), m_convo);
    }

    return success;
}

}
