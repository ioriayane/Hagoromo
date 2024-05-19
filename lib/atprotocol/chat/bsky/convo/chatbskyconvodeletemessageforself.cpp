#include "chatbskyconvodeletemessageforself.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyConvoDeleteMessageForSelf::ChatBskyConvoDeleteMessageForSelf(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyConvoDeleteMessageForSelf::deleteMessageForSelf(const QString &convoId,
                                                             const QString &messageId)
{
    QJsonObject json_obj;
    if (!convoId.isEmpty()) {
        json_obj.insert(QStringLiteral("convoId"), convoId);
    }
    if (!messageId.isEmpty()) {
        json_obj.insert(QStringLiteral("messageId"), messageId);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.convo.deleteMessageForSelf"),
         json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::ChatBskyConvoDefs::DeletedMessageView &
ChatBskyConvoDeleteMessageForSelf::deletedMessageView() const
{
    return m_deletedMessageView;
}

bool ChatBskyConvoDeleteMessageForSelf::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::ChatBskyConvoDefs::copyDeletedMessageView(json_doc.object(),
                                                                  m_deletedMessageView);
    }

    return success;
}

}
