#include "chatbskyactorgetstatus.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyActorGetStatus::ChatBskyActorGetStatus(QObject *parent) : AccessAtProtocol { parent } { }

void ChatBskyActorGetStatus::getStatus()
{
    QUrlQuery url_query;

    get(QStringLiteral("xrpc/chat.bsky.actor.getStatus"), url_query);
}

const bool &ChatBskyActorGetStatus::chatDisabled() const
{
    return m_chatDisabled;
}

const bool &ChatBskyActorGetStatus::canCreateGroups() const
{
    return m_canCreateGroups;
}

const int &ChatBskyActorGetStatus::groupMemberLimit() const
{
    return m_groupMemberLimit;
}

bool ChatBskyActorGetStatus::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyBool(json_doc.object().value("chatDisabled"),
                                                      m_chatDisabled);
        AtProtocolType::LexiconsTypeUnknown::copyBool(json_doc.object().value("canCreateGroups"),
                                                      m_canCreateGroups);
        AtProtocolType::LexiconsTypeUnknown::copyInt(json_doc.object().value("groupMemberLimit"),
                                                     m_groupMemberLimit);
    }

    return success;
}

}
