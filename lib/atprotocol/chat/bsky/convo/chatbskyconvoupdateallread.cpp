#include "chatbskyconvoupdateallread.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyConvoUpdateAllRead::ChatBskyConvoUpdateAllRead(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyConvoUpdateAllRead::updateAllRead(const QString &status)
{
    appendRawHeader("atproto-proxy", "did:web:api.bsky.chat#bsky_chat");

    QJsonObject json_obj;
    if (!status.isEmpty()) {
        json_obj.insert(QStringLiteral("status"), status);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.convo.updateAllRead"),
         json_doc.toJson(QJsonDocument::Compact));
}

const int &ChatBskyConvoUpdateAllRead::updatedCount() const
{
    return m_updatedCount;
}

bool ChatBskyConvoUpdateAllRead::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyInt(json_doc.object().value("updatedCount"),
                                                     m_updatedCount);
    }

    return success;
}

}
