#include "chatbskyconvogetmessages.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyConvoGetMessages::ChatBskyConvoGetMessages(QObject *parent)
    : AccessAtProtocol { parent } { }

void ChatBskyConvoGetMessages::getMessages(const QString &convoId, const qint64 limit,
                                           const QString &cursor)
{
    QUrlQuery url_query;
    if (!convoId.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("convoId"), convoId);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/chat.bsky.convo.getMessages"), url_query);
}

const QList<AtProtocolType::ChatBskyConvoDefs::MessageView> &
ChatBskyConvoGetMessages::messagesMessageViewList() const
{
    return m_messagesMessageViewList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::DeletedMessageView> &
ChatBskyConvoGetMessages::messagesDeletedMessageViewList() const
{
    return m_messagesDeletedMessageViewList;
}

bool ChatBskyConvoGetMessages::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("messages")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        QString type;
        for (const auto &value : json_doc.object().value("messages").toArray()) {
            type = value.toObject().value("$type").toString();
            if (type == QStringLiteral("chat.bsky.convo.defs#messageView")) {
                AtProtocolType::ChatBskyConvoDefs::MessageView data;
                AtProtocolType::ChatBskyConvoDefs::copyMessageView(value.toObject(), data);
                m_messagesMessageViewList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#deletedMessageView")) {
                AtProtocolType::ChatBskyConvoDefs::DeletedMessageView data;
                AtProtocolType::ChatBskyConvoDefs::copyDeletedMessageView(value.toObject(), data);
                m_messagesDeletedMessageViewList.append(data);
            }
        }
    }

    return success;
}

}
