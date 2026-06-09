#include "chatbskygroupgetjoinlinkpreviews.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyGroupGetJoinLinkPreviews::ChatBskyGroupGetJoinLinkPreviews(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyGroupGetJoinLinkPreviews::getJoinLinkPreviews(const QList<QString> &codes)
{
    QUrlQuery url_query;
    for (const auto &value : codes) {
        url_query.addQueryItem(QStringLiteral("codes"), value);
    }

    get(QStringLiteral("xrpc/chat.bsky.group.getJoinLinkPreviews"), url_query);
}

const QList<AtProtocolType::ChatBskyGroupDefs::JoinLinkPreviewView> &
ChatBskyGroupGetJoinLinkPreviews::joinLinkPreviewsJoinLinkPreviewViewList() const
{
    return m_joinLinkPreviewsJoinLinkPreviewViewList;
}

const QList<AtProtocolType::ChatBskyGroupDefs::DisabledJoinLinkPreviewView> &
ChatBskyGroupGetJoinLinkPreviews::joinLinkPreviewsDisabledJoinLinkPreviewViewList() const
{
    return m_joinLinkPreviewsDisabledJoinLinkPreviewViewList;
}

const QList<AtProtocolType::ChatBskyGroupDefs::InvalidJoinLinkPreviewView> &
ChatBskyGroupGetJoinLinkPreviews::joinLinkPreviewsInvalidJoinLinkPreviewViewList() const
{
    return m_joinLinkPreviewsInvalidJoinLinkPreviewViewList;
}

bool ChatBskyGroupGetJoinLinkPreviews::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("joinLinkPreviews")) {
        success = false;
    } else {
        QString type;
        for (const auto &value : json_doc.object().value("joinLinkPreviews").toArray()) {
            type = value.toObject().value("$type").toString();
            if (type == QStringLiteral("chat.bsky.group.defs#joinLinkPreviewView")) {
                AtProtocolType::ChatBskyGroupDefs::JoinLinkPreviewView data;
                AtProtocolType::ChatBskyGroupDefs::copyJoinLinkPreviewView(value.toObject(), data);
                m_joinLinkPreviewsJoinLinkPreviewViewList.append(data);
            } else if (type == QStringLiteral("chat.bsky.group.defs#disabledJoinLinkPreviewView")) {
                AtProtocolType::ChatBskyGroupDefs::DisabledJoinLinkPreviewView data;
                AtProtocolType::ChatBskyGroupDefs::copyDisabledJoinLinkPreviewView(value.toObject(),
                                                                                   data);
                m_joinLinkPreviewsDisabledJoinLinkPreviewViewList.append(data);
            } else if (type == QStringLiteral("chat.bsky.group.defs#invalidJoinLinkPreviewView")) {
                AtProtocolType::ChatBskyGroupDefs::InvalidJoinLinkPreviewView data;
                AtProtocolType::ChatBskyGroupDefs::copyInvalidJoinLinkPreviewView(value.toObject(),
                                                                                  data);
                m_joinLinkPreviewsInvalidJoinLinkPreviewViewList.append(data);
            }
        }
    }

    return success;
}

}
