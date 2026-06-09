#include "chatbskymoderationgetconvo.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyModerationGetConvo::ChatBskyModerationGetConvo(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyModerationGetConvo::getConvo(const QString &convoId)
{
    QUrlQuery url_query;
    if (!convoId.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("convoId"), convoId);
    }

    get(QStringLiteral("xrpc/chat.bsky.moderation.getConvo"), url_query);
}

const AtProtocolType::ChatBskyModerationDefs::ConvoView &ChatBskyModerationGetConvo::convo() const
{
    return m_convo;
}

bool ChatBskyModerationGetConvo::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::ChatBskyModerationDefs::copyConvoView(
                json_doc.object().value("convo").toObject(), m_convo);
    }

    return success;
}

}
