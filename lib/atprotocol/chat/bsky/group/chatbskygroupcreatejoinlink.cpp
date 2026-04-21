#include "chatbskygroupcreatejoinlink.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyGroupCreateJoinLink::ChatBskyGroupCreateJoinLink(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyGroupCreateJoinLink::createJoinLink(const QString &convoId, const bool requireApproval,
                                                 const QString &joinRule)
{
    QJsonObject json_obj;
    if (!convoId.isEmpty()) {
        json_obj.insert(QStringLiteral("convoId"), convoId);
    }
    json_obj.insert(QStringLiteral("requireApproval"), requireApproval);
    if (!joinRule.isEmpty()) {
        json_obj.insert(QStringLiteral("joinRule"), joinRule);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.group.createJoinLink"),
         json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::ChatBskyGroupDefs::JoinLinkView &ChatBskyGroupCreateJoinLink::joinLink() const
{
    return m_joinLink;
}

bool ChatBskyGroupCreateJoinLink::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::ChatBskyGroupDefs::copyJoinLinkView(
                json_doc.object().value("joinLink").toObject(), m_joinLink);
    }

    return success;
}

}
