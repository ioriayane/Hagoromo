#include "chatbskyconvosendmessage.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyConvoSendMessage::ChatBskyConvoSendMessage(QObject *parent)
    : AccessAtProtocol { parent } { }

void ChatBskyConvoSendMessage::sendMessage(const QString &convoId, const QJsonObject &message)
{
    appendRawHeader("atproto-proxy", "did:web:api.bsky.chat#bsky_chat");

    QJsonObject json_obj;
    if (!convoId.isEmpty()) {
        json_obj.insert(QStringLiteral("convoId"), convoId);
    }
    if (!message.isEmpty()) {
        json_obj.insert(QStringLiteral("message"), message);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.convo.sendMessage"),
         json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::ChatBskyConvoDefs::MessageView &ChatBskyConvoSendMessage::messageView() const
{
    return m_messageView;
}

bool ChatBskyConvoSendMessage::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::ChatBskyConvoDefs::copyMessageView(json_doc.object(), m_messageView);
    }

    return success;
}

}
