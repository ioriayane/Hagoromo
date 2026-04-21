#include "chatbskygrouprequestjoin.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ChatBskyGroupRequestJoin::ChatBskyGroupRequestJoin(QObject *parent)
    : AccessAtProtocol { parent } { }

void ChatBskyGroupRequestJoin::requestJoin(const QString &code)
{
    QJsonObject json_obj;
    if (!code.isEmpty()) {
        json_obj.insert(QStringLiteral("code"), code);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/chat.bsky.group.requestJoin"),
         json_doc.toJson(QJsonDocument::Compact));
}

const QString &ChatBskyGroupRequestJoin::status() const
{
    return m_status;
}

const AtProtocolType::ChatBskyConvoDefs::ConvoView &ChatBskyGroupRequestJoin::convo() const
{
    return m_convo;
}

bool ChatBskyGroupRequestJoin::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("status"),
                                                        m_status);
        AtProtocolType::ChatBskyConvoDefs::copyConvoView(
                json_doc.object().value("convo").toObject(), m_convo);
    }

    return success;
}

}
