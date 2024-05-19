#include "chatbskyconvosendmessagebatch.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyConvoSendMessageBatch::ChatBskyConvoSendMessageBatch(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyConvoSendMessageBatch::sendMessageBatch(const QJsonObject &items)
{
    QJsonObject json_obj;
    if (!items.isEmpty()) {
        json_obj.insert(QStringLiteral("items"), items);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.convo.sendMessageBatch"),
         json_doc.toJson(QJsonDocument::Compact));
}

const QList<AtProtocolType::ChatBskyConvoDefs::MessageView> &
ChatBskyConvoSendMessageBatch::itemsMessageViewList() const
{
    return m_itemsMessageViewList;
}

bool ChatBskyConvoSendMessageBatch::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("items")) {
        success = false;
    } else {
        for (const auto &value : json_doc.object().value("items").toArray()) {
            AtProtocolType::ChatBskyConvoDefs::MessageView data;
            AtProtocolType::ChatBskyConvoDefs::copyMessageView(value.toObject(), data);
            m_itemsMessageViewList.append(data);
        }
    }

    return success;
}

}
