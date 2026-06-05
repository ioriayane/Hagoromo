#include "chatbskyconvounlockconvo.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyConvoUnlockConvo::ChatBskyConvoUnlockConvo(QObject *parent)
    : AccessAtProtocol { parent } { }

void ChatBskyConvoUnlockConvo::unlockConvo(const QString &convoId)
{
    QJsonObject json_obj;
    if (!convoId.isEmpty()) {
        json_obj.insert(QStringLiteral("convoId"), convoId);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.convo.unlockConvo"),
         json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::ChatBskyConvoDefs::ConvoView &ChatBskyConvoUnlockConvo::convo() const
{
    return m_convo;
}

bool ChatBskyConvoUnlockConvo::parseJson(bool success, const QString reply_json)
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
