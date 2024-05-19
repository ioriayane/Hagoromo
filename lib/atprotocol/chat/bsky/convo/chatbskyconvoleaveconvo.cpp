#include "chatbskyconvoleaveconvo.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyConvoLeaveConvo::ChatBskyConvoLeaveConvo(QObject *parent) : AccessAtProtocol { parent } { }

void ChatBskyConvoLeaveConvo::leaveConvo(const QString &convoId)
{
    QJsonObject json_obj;
    if (!convoId.isEmpty()) {
        json_obj.insert(QStringLiteral("convoId"), convoId);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.convo.leaveConvo"),
         json_doc.toJson(QJsonDocument::Compact));
}

const QString &ChatBskyConvoLeaveConvo::convoId() const
{
    return m_convoId;
}

const QString &ChatBskyConvoLeaveConvo::rev() const
{
    return m_rev;
}

bool ChatBskyConvoLeaveConvo::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("convoId"),
                                                        m_convoId);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("rev"), m_rev);
    }

    return success;
}

}
