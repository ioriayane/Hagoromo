#include "chatbskygroupdisablejoinlink.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyGroupDisableJoinLink::ChatBskyGroupDisableJoinLink(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyGroupDisableJoinLink::disableJoinLink(const QString &convoId)
{
    QJsonObject json_obj;
    if (!convoId.isEmpty()) {
        json_obj.insert(QStringLiteral("convoId"), convoId);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.group.disableJoinLink"),
         json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::ChatBskyGroupDefs::JoinLinkView &
ChatBskyGroupDisableJoinLink::joinLink() const
{
    return m_joinLink;
}

bool ChatBskyGroupDisableJoinLink::parseJson(bool success, const QString reply_json)
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
