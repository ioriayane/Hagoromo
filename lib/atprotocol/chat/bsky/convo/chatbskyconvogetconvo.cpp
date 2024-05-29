#include "chatbskyconvogetconvo.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyConvoGetConvo::ChatBskyConvoGetConvo(QObject *parent) : AccessAtProtocol { parent } { }

void ChatBskyConvoGetConvo::getConvo(const QString &convoId)
{
    appendRawHeader("atproto-proxy", "did:web:api.bsky.chat#bsky_chat");

    QUrlQuery url_query;
    if (!convoId.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("convoId"), convoId);
    }

    get(QStringLiteral("xrpc/chat.bsky.convo.getConvo"), url_query);
}

const AtProtocolType::ChatBskyConvoDefs::ConvoView &ChatBskyConvoGetConvo::convo() const
{
    return m_convo;
}

bool ChatBskyConvoGetConvo::parseJson(bool success, const QString reply_json)
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
