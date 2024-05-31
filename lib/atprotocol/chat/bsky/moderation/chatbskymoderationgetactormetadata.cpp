#include "chatbskymoderationgetactormetadata.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyModerationGetActorMetadata::ChatBskyModerationGetActorMetadata(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyModerationGetActorMetadata::getActorMetadata(const QString &actor)
{
    appendRawHeader("atproto-proxy", "did:web:api.bsky.chat#bsky_chat");

    QUrlQuery url_query;
    if (!actor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("actor"), actor);
    }

    get(QStringLiteral("xrpc/chat.bsky.moderation.getActorMetadata"), url_query);
}

const AtProtocolType::ChatBskyModerationGetActorMetadata::Metadata &
ChatBskyModerationGetActorMetadata::day() const
{
    return m_day;
}

const AtProtocolType::ChatBskyModerationGetActorMetadata::Metadata &
ChatBskyModerationGetActorMetadata::month() const
{
    return m_month;
}

const AtProtocolType::ChatBskyModerationGetActorMetadata::Metadata &
ChatBskyModerationGetActorMetadata::all() const
{
    return m_all;
}

bool ChatBskyModerationGetActorMetadata::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::ChatBskyModerationGetActorMetadata::copyMetadata(
                json_doc.object().value("day").toObject(), m_day);
        AtProtocolType::ChatBskyModerationGetActorMetadata::copyMetadata(
                json_doc.object().value("month").toObject(), m_month);
        AtProtocolType::ChatBskyModerationGetActorMetadata::copyMetadata(
                json_doc.object().value("all").toObject(), m_all);
    }

    return success;
}

}
