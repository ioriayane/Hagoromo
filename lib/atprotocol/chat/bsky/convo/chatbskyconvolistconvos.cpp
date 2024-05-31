#include "chatbskyconvolistconvos.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyConvoListConvos::ChatBskyConvoListConvos(QObject *parent) : AccessAtProtocol { parent } { }

void ChatBskyConvoListConvos::listConvos(const int limit, const QString &cursor)
{
    appendRawHeader("atproto-proxy", "did:web:api.bsky.chat#bsky_chat");

    QUrlQuery url_query;
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/chat.bsky.convo.listConvos"), url_query);
}

const QList<AtProtocolType::ChatBskyConvoDefs::ConvoView> &
ChatBskyConvoListConvos::convosList() const
{
    return m_convosList;
}

bool ChatBskyConvoListConvos::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("convos")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &value : json_doc.object().value("convos").toArray()) {
            AtProtocolType::ChatBskyConvoDefs::ConvoView data;
            AtProtocolType::ChatBskyConvoDefs::copyConvoView(value.toObject(), data);
            m_convosList.append(data);
        }
    }

    return success;
}

}
