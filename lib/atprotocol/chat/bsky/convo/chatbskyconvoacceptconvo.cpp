#include "chatbskyconvoacceptconvo.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyConvoAcceptConvo::ChatBskyConvoAcceptConvo(QObject *parent)
    : AccessAtProtocol { parent } { }

void ChatBskyConvoAcceptConvo::acceptConvo(const QString &convoId)
{
    appendRawHeader("atproto-proxy", "did:web:api.bsky.chat#bsky_chat");

    QJsonObject json_obj;
    if (!convoId.isEmpty()) {
        json_obj.insert(QStringLiteral("convoId"), convoId);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.convo.acceptConvo"),
         json_doc.toJson(QJsonDocument::Compact));
}

const QString &ChatBskyConvoAcceptConvo::rev() const
{
    return m_rev;
}

bool ChatBskyConvoAcceptConvo::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("rev"), m_rev);
    }

    return success;
}

}
