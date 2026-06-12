#include "chatbskyconvogetconvomembers.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyConvoGetConvoMembers::ChatBskyConvoGetConvoMembers(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyConvoGetConvoMembers::getConvoMembers(const QString &convoId, const int limit,
                                                   const QString &cursor)
{
    QUrlQuery url_query;
    if (!convoId.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("convoId"), convoId);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/chat.bsky.convo.getConvoMembers"), url_query);
}

const QList<AtProtocolType::ChatBskyActorDefs::ProfileViewBasic> &
ChatBskyConvoGetConvoMembers::membersList() const
{
    return m_membersList;
}

bool ChatBskyConvoGetConvoMembers::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("members")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &value : json_doc.object().value("members").toArray()) {
            AtProtocolType::ChatBskyActorDefs::ProfileViewBasic data;
            AtProtocolType::ChatBskyActorDefs::copyProfileViewBasic(value.toObject(), data);
            m_membersList.append(data);
        }
    }

    return success;
}

}
