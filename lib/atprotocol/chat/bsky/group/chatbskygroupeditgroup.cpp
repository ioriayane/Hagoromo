#include "chatbskygroupeditgroup.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyGroupEditGroup::ChatBskyGroupEditGroup(QObject *parent) : AccessAtProtocol { parent } { }

void ChatBskyGroupEditGroup::editGroup(const QString &convoId, const QString &name)
{
    QJsonObject json_obj;
    if (!convoId.isEmpty()) {
        json_obj.insert(QStringLiteral("convoId"), convoId);
    }
    if (!name.isEmpty()) {
        json_obj.insert(QStringLiteral("name"), name);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.group.editGroup"), json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::ChatBskyConvoDefs::ConvoView &ChatBskyGroupEditGroup::convo() const
{
    return m_convo;
}

bool ChatBskyGroupEditGroup::parseJson(bool success, const QString reply_json)
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
