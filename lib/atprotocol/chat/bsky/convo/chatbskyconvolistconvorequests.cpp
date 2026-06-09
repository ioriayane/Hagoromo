#include "chatbskyconvolistconvorequests.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyConvoListConvoRequests::ChatBskyConvoListConvoRequests(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyConvoListConvoRequests::listConvoRequests(const int limit, const QString &cursor)
{
    QUrlQuery url_query;
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/chat.bsky.convo.listConvoRequests"), url_query);
}

const QList<AtProtocolType::ChatBskyConvoDefs::ConvoView> &
ChatBskyConvoListConvoRequests::requestsConvoViewList() const
{
    return m_requestsConvoViewList;
}

const QList<AtProtocolType::ChatBskyGroupDefs::JoinRequestConvoView> &
ChatBskyConvoListConvoRequests::requestsJoinRequestConvoViewList() const
{
    return m_requestsJoinRequestConvoViewList;
}

bool ChatBskyConvoListConvoRequests::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("requests")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        QString type;
        for (const auto &value : json_doc.object().value("requests").toArray()) {
            type = value.toObject().value("$type").toString();
            if (type == QStringLiteral("chat.bsky.convo.defs#convoView")) {
                AtProtocolType::ChatBskyConvoDefs::ConvoView data;
                AtProtocolType::ChatBskyConvoDefs::copyConvoView(value.toObject(), data);
                m_requestsConvoViewList.append(data);
            } else if (type == QStringLiteral("chat.bsky.group.defs#joinRequestConvoView")) {
                AtProtocolType::ChatBskyGroupDefs::JoinRequestConvoView data;
                AtProtocolType::ChatBskyGroupDefs::copyJoinRequestConvoView(value.toObject(), data);
                m_requestsJoinRequestConvoViewList.append(data);
            }
        }
    }

    return success;
}

}
