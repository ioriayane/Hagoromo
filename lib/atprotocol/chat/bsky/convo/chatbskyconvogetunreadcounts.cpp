#include "chatbskyconvogetunreadcounts.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyConvoGetUnreadCounts::ChatBskyConvoGetUnreadCounts(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyConvoGetUnreadCounts::getUnreadCounts()
{
    QUrlQuery url_query;

    get(QStringLiteral("xrpc/chat.bsky.convo.getUnreadCounts"), url_query);
}

const int &ChatBskyConvoGetUnreadCounts::unreadAcceptedConvos() const
{
    return m_unreadAcceptedConvos;
}

const int &ChatBskyConvoGetUnreadCounts::unreadRequestConvos() const
{
    return m_unreadRequestConvos;
}

bool ChatBskyConvoGetUnreadCounts::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyInt(
                json_doc.object().value("unreadAcceptedConvos"), m_unreadAcceptedConvos);
        AtProtocolType::LexiconsTypeUnknown::copyInt(json_doc.object().value("unreadRequestConvos"),
                                                     m_unreadRequestConvos);
    }

    return success;
}

}
