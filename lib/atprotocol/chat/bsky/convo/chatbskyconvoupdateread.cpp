#include "chatbskyconvoupdateread.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyConvoUpdateRead::ChatBskyConvoUpdateRead(QObject *parent) : AccessAtProtocol { parent } { }

void ChatBskyConvoUpdateRead::updateRead(const QString &convoId, const QString &messageId)
{
    QJsonObject json_obj;
    if (!convoId.isEmpty()) {
        json_obj.insert(QStringLiteral("convoId"), convoId);
    }
    if (!messageId.isEmpty()) {
        json_obj.insert(QStringLiteral("messageId"), messageId);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.convo.updateRead"),
         json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::ChatBskyConvoDefs::ConvoView &ChatBskyConvoUpdateRead::convo() const
{
    return m_convo;
}

bool ChatBskyConvoUpdateRead::parseJson(bool success, const QString reply_json)
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
