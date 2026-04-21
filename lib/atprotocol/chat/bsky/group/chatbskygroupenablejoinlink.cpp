#include "chatbskygroupenablejoinlink.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyGroupEnableJoinLink::ChatBskyGroupEnableJoinLink(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyGroupEnableJoinLink::enableJoinLink(const QString &convoId)
{
    QJsonObject json_obj;
    if (!convoId.isEmpty()) {
        json_obj.insert(QStringLiteral("convoId"), convoId);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.group.enableJoinLink"),
         json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::ChatBskyGroupDefs::JoinLinkView &ChatBskyGroupEnableJoinLink::joinLink() const
{
    return m_joinLink;
}

bool ChatBskyGroupEnableJoinLink::parseJson(bool success, const QString reply_json)
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
