#include "chatbskygroupremovemembers.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyGroupRemoveMembers::ChatBskyGroupRemoveMembers(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyGroupRemoveMembers::removeMembers(const QString &convoId,
                                               const QList<QString> &members)
{
    QJsonObject json_obj;
    if (!convoId.isEmpty()) {
        json_obj.insert(QStringLiteral("convoId"), convoId);
    }
    for (const auto &value : members) {
        url_query.addQueryItem(QStringLiteral("members"), value);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.group.removeMembers"),
         json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::ChatBskyConvoDefs::ConvoView &ChatBskyGroupRemoveMembers::convo() const
{
    return m_convo;
}

bool ChatBskyGroupRemoveMembers::parseJson(bool success, const QString reply_json)
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
