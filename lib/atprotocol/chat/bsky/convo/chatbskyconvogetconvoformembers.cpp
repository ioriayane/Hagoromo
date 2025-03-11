#include "chatbskyconvogetconvoformembers.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyConvoGetConvoForMembers::ChatBskyConvoGetConvoForMembers(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyConvoGetConvoForMembers::getConvoForMembers(const QList<QString> &members)
{
    QUrlQuery url_query;
    for (const auto &value : members) {
        url_query.addQueryItem(QStringLiteral("members"), value);
    }

    get(QStringLiteral("xrpc/chat.bsky.convo.getConvoForMembers"), url_query);
}

const AtProtocolType::ChatBskyConvoDefs::ConvoView &ChatBskyConvoGetConvoForMembers::convo() const
{
    return m_convo;
}

bool ChatBskyConvoGetConvoForMembers::parseJson(bool success, const QString reply_json)
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
