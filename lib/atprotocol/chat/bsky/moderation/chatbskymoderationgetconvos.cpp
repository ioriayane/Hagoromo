#include "chatbskymoderationgetconvos.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyModerationGetConvos::ChatBskyModerationGetConvos(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyModerationGetConvos::getConvos(const QList<QString> &convoIds)
{
    QUrlQuery url_query;
    for (const auto &value : convoIds) {
        url_query.addQueryItem(QStringLiteral("convoIds"), value);
    }

    get(QStringLiteral("xrpc/chat.bsky.moderation.getConvos"), url_query);
}

const QList<AtProtocolType::ChatBskyModerationDefs::ConvoView> &
ChatBskyModerationGetConvos::convosList() const
{
    return m_convosList;
}

bool ChatBskyModerationGetConvos::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("convos")) {
        success = false;
    } else {
        for (const auto &value : json_doc.object().value("convos").toArray()) {
            AtProtocolType::ChatBskyModerationDefs::ConvoView data;
            AtProtocolType::ChatBskyModerationDefs::copyConvoView(value.toObject(), data);
            m_convosList.append(data);
        }
    }

    return success;
}

}
