#include "chatbskymoderationupdateactoraccess.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyModerationUpdateActorAccess::ChatBskyModerationUpdateActorAccess(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyModerationUpdateActorAccess::updateActorAccess(const QString &actor,
                                                            const bool allowAccess,
                                                            const QString &ref)
{
    QJsonObject json_obj;
    if (!actor.isEmpty()) {
        json_obj.insert(QStringLiteral("actor"), actor);
    }
    json_obj.insert(QStringLiteral("allowAccess"), allowAccess);
    if (!ref.isEmpty()) {
        json_obj.insert(QStringLiteral("ref"), ref);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.moderation.updateActorAccess"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool ChatBskyModerationUpdateActorAccess::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
