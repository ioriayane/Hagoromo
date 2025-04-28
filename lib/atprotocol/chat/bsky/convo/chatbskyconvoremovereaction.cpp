#include "chatbskyconvoremovereaction.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyConvoRemoveReaction::ChatBskyConvoRemoveReaction(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyConvoRemoveReaction::removeReaction(const QString &convoId, const QString &messageId,
                                                 const QString &value)
{
    QJsonObject json_obj;
    if (!convoId.isEmpty()) {
        json_obj.insert(QStringLiteral("convoId"), convoId);
    }
    if (!messageId.isEmpty()) {
        json_obj.insert(QStringLiteral("messageId"), messageId);
    }
    if (!value.isEmpty()) {
        json_obj.insert(QStringLiteral("value"), value);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.convo.removeReaction"),
         json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::ChatBskyConvoDefs::MessageView &ChatBskyConvoRemoveReaction::message() const
{
    return m_message;
}

bool ChatBskyConvoRemoveReaction::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::ChatBskyConvoDefs::copyMessageView(
                json_doc.object().value("message").toObject(), m_message);
    }

    return success;
}

}
