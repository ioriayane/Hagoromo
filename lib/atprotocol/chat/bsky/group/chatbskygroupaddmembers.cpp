#include "chatbskygroupaddmembers.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyGroupAddMembers::ChatBskyGroupAddMembers(QObject *parent) : AccessAtProtocol { parent } { }

void ChatBskyGroupAddMembers::addMembers(const QString &convoId, const QList<QString> &members)
{
    QJsonObject json_obj;
    if (!convoId.isEmpty()) {
        json_obj.insert(QStringLiteral("convoId"), convoId);
    }
    for (const auto &value : members) {
        url_query.addQueryItem(QStringLiteral("members"), value);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.group.addMembers"),
         json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::ChatBskyConvoDefs::ConvoView &ChatBskyGroupAddMembers::convo() const
{
    return m_convo;
}

const QList<AtProtocolType::ChatBskyActorDefs::ProfileViewBasic> &
ChatBskyGroupAddMembers::addedMembersList() const
{
    return m_addedMembersList;
}

bool ChatBskyGroupAddMembers::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("addedMembers")) {
        success = false;
    } else {
        AtProtocolType::ChatBskyConvoDefs::copyConvoView(
                json_doc.object().value("convo").toObject(), m_convo);
        for (const auto &value : json_doc.object().value("addedMembers").toArray()) {
            AtProtocolType::ChatBskyActorDefs::ProfileViewBasic data;
            AtProtocolType::ChatBskyActorDefs::copyProfileViewBasic(value.toObject(), data);
            m_addedMembersList.append(data);
        }
    }

    return success;
}

}
