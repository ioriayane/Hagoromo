#include "chatbskymoderationgetmessagecontext.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyModerationGetMessageContext::ChatBskyModerationGetMessageContext(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyModerationGetMessageContext::getMessageContext(const QString &convoId,
                                                            const QString &messageId,
                                                            const int before, const int after,
                                                            const int maxInterleavedSystemMessages)
{
    QUrlQuery url_query;
    if (!convoId.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("convoId"), convoId);
    }
    if (!messageId.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("messageId"), messageId);
    }
    if (before > 0) {
        url_query.addQueryItem(QStringLiteral("before"), QString::number(before));
    }
    if (after > 0) {
        url_query.addQueryItem(QStringLiteral("after"), QString::number(after));
    }
    if (maxInterleavedSystemMessages > 0) {
        url_query.addQueryItem(QStringLiteral("maxInterleavedSystemMessages"),
                               QString::number(maxInterleavedSystemMessages));
    }

    get(QStringLiteral("xrpc/chat.bsky.moderation.getMessageContext"), url_query);
}

const QList<AtProtocolType::ChatBskyConvoDefs::MessageView> &
ChatBskyModerationGetMessageContext::messagesMessageViewList() const
{
    return m_messagesMessageViewList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::SystemMessageView> &
ChatBskyModerationGetMessageContext::messagesSystemMessageViewList() const
{
    return m_messagesSystemMessageViewList;
}

bool ChatBskyModerationGetMessageContext::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("messages")) {
        success = false;
    } else {
        QString type;
        for (const auto &value : json_doc.object().value("messages").toArray()) {
            type = value.toObject().value("$type").toString();
            if (type == QStringLiteral("chat.bsky.convo.defs#messageView")) {
                AtProtocolType::ChatBskyConvoDefs::MessageView data;
                AtProtocolType::ChatBskyConvoDefs::copyMessageView(value.toObject(), data);
                m_messagesMessageViewList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#systemMessageView")) {
                AtProtocolType::ChatBskyConvoDefs::SystemMessageView data;
                AtProtocolType::ChatBskyConvoDefs::copySystemMessageView(value.toObject(), data);
                m_messagesSystemMessageViewList.append(data);
            }
        }
    }

    return success;
}

}
