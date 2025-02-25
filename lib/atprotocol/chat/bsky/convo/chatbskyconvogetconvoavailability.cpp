#include "chatbskyconvogetconvoavailability.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyConvoGetConvoAvailability::ChatBskyConvoGetConvoAvailability(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyConvoGetConvoAvailability::getConvoAvailability(const QList<QString> &members)
{
    appendRawHeader("atproto-proxy", "did:web:api.bsky.chat#bsky_chat");

    QUrlQuery url_query;
    for (const auto &value : members) {
        url_query.addQueryItem(QStringLiteral("members"), value);
    }

    get(QStringLiteral("xrpc/chat.bsky.convo.getConvoAvailability"), url_query);
}

const bool &ChatBskyConvoGetConvoAvailability::canChat() const
{
    return m_canChat;
}

const AtProtocolType::ChatBskyConvoDefs::ConvoView &ChatBskyConvoGetConvoAvailability::convo() const
{
    return m_convo;
}

bool ChatBskyConvoGetConvoAvailability::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyBool(json_doc.object().value("canChat"),
                                                      m_canChat);
        AtProtocolType::ChatBskyConvoDefs::copyConvoView(
                json_doc.object().value("convo").toObject(), m_convo);
    }

    return success;
}

}
