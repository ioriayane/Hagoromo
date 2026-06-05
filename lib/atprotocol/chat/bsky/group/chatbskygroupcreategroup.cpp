#include "chatbskygroupcreategroup.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyGroupCreateGroup::ChatBskyGroupCreateGroup(QObject *parent)
    : AccessAtProtocol { parent } { }

void ChatBskyGroupCreateGroup::createGroup(const QList<QString> &members, const QString &name)
{
    QJsonObject json_obj;
    for (const auto &value : members) {
        url_query.addQueryItem(QStringLiteral("members"), value);
    }
    if (!name.isEmpty()) {
        json_obj.insert(QStringLiteral("name"), name);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.group.createGroup"),
         json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::ChatBskyConvoDefs::ConvoView &ChatBskyGroupCreateGroup::convo() const
{
    return m_convo;
}

bool ChatBskyGroupCreateGroup::parseJson(bool success, const QString reply_json)
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
