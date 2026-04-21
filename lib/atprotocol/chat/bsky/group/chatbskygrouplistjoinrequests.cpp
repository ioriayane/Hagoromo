#include "chatbskygrouplistjoinrequests.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyGroupListJoinRequests::ChatBskyGroupListJoinRequests(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyGroupListJoinRequests::listJoinRequests(const QString &convoId, const int limit,
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

    get(QStringLiteral("xrpc/chat.bsky.group.listJoinRequests"), url_query);
}

const QList<AtProtocolType::ChatBskyGroupDefs::JoinRequestView> &
ChatBskyGroupListJoinRequests::requestsList() const
{
    return m_requestsList;
}

bool ChatBskyGroupListJoinRequests::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("requests")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &value : json_doc.object().value("requests").toArray()) {
            AtProtocolType::ChatBskyGroupDefs::JoinRequestView data;
            AtProtocolType::ChatBskyGroupDefs::copyJoinRequestView(value.toObject(), data);
            m_requestsList.append(data);
        }
    }

    return success;
}

}
