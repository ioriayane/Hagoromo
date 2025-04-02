#include "chatbskyconvogetlog.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyConvoGetLog::ChatBskyConvoGetLog(QObject *parent) : AccessAtProtocol { parent } { }

void ChatBskyConvoGetLog::getLog(const QString &cursor)
{
    QUrlQuery url_query;
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/chat.bsky.convo.getLog"), url_query);
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogBeginConvo> &
ChatBskyConvoGetLog::logsLogBeginConvoList() const
{
    return m_logsLogBeginConvoList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogAcceptConvo> &
ChatBskyConvoGetLog::logsLogAcceptConvoList() const
{
    return m_logsLogAcceptConvoList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogLeaveConvo> &
ChatBskyConvoGetLog::logsLogLeaveConvoList() const
{
    return m_logsLogLeaveConvoList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogMuteConvo> &
ChatBskyConvoGetLog::logsLogMuteConvoList() const
{
    return m_logsLogMuteConvoList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogUnmuteConvo> &
ChatBskyConvoGetLog::logsLogUnmuteConvoList() const
{
    return m_logsLogUnmuteConvoList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogCreateMessage> &
ChatBskyConvoGetLog::logsLogCreateMessageList() const
{
    return m_logsLogCreateMessageList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogDeleteMessage> &
ChatBskyConvoGetLog::logsLogDeleteMessageList() const
{
    return m_logsLogDeleteMessageList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogReadMessage> &
ChatBskyConvoGetLog::logsLogReadMessageList() const
{
    return m_logsLogReadMessageList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogAddReaction> &
ChatBskyConvoGetLog::logsLogAddReactionList() const
{
    return m_logsLogAddReactionList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogRemoveReaction> &
ChatBskyConvoGetLog::logsLogRemoveReactionList() const
{
    return m_logsLogRemoveReactionList;
}

bool ChatBskyConvoGetLog::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("logs")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        QString type;
        for (const auto &value : json_doc.object().value("logs").toArray()) {
            type = value.toObject().value("$type").toString();
            if (type == QStringLiteral("chat.bsky.convo.defs#logBeginConvo")) {
                AtProtocolType::ChatBskyConvoDefs::LogBeginConvo data;
                AtProtocolType::ChatBskyConvoDefs::copyLogBeginConvo(value.toObject(), data);
                m_logsLogBeginConvoList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logAcceptConvo")) {
                AtProtocolType::ChatBskyConvoDefs::LogAcceptConvo data;
                AtProtocolType::ChatBskyConvoDefs::copyLogAcceptConvo(value.toObject(), data);
                m_logsLogAcceptConvoList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logLeaveConvo")) {
                AtProtocolType::ChatBskyConvoDefs::LogLeaveConvo data;
                AtProtocolType::ChatBskyConvoDefs::copyLogLeaveConvo(value.toObject(), data);
                m_logsLogLeaveConvoList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logMuteConvo")) {
                AtProtocolType::ChatBskyConvoDefs::LogMuteConvo data;
                AtProtocolType::ChatBskyConvoDefs::copyLogMuteConvo(value.toObject(), data);
                m_logsLogMuteConvoList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logUnmuteConvo")) {
                AtProtocolType::ChatBskyConvoDefs::LogUnmuteConvo data;
                AtProtocolType::ChatBskyConvoDefs::copyLogUnmuteConvo(value.toObject(), data);
                m_logsLogUnmuteConvoList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logCreateMessage")) {
                AtProtocolType::ChatBskyConvoDefs::LogCreateMessage data;
                AtProtocolType::ChatBskyConvoDefs::copyLogCreateMessage(value.toObject(), data);
                m_logsLogCreateMessageList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logDeleteMessage")) {
                AtProtocolType::ChatBskyConvoDefs::LogDeleteMessage data;
                AtProtocolType::ChatBskyConvoDefs::copyLogDeleteMessage(value.toObject(), data);
                m_logsLogDeleteMessageList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logReadMessage")) {
                AtProtocolType::ChatBskyConvoDefs::LogReadMessage data;
                AtProtocolType::ChatBskyConvoDefs::copyLogReadMessage(value.toObject(), data);
                m_logsLogReadMessageList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logAddReaction")) {
                AtProtocolType::ChatBskyConvoDefs::LogAddReaction data;
                AtProtocolType::ChatBskyConvoDefs::copyLogAddReaction(value.toObject(), data);
                m_logsLogAddReactionList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logRemoveReaction")) {
                AtProtocolType::ChatBskyConvoDefs::LogRemoveReaction data;
                AtProtocolType::ChatBskyConvoDefs::copyLogRemoveReaction(value.toObject(), data);
                m_logsLogRemoveReactionList.append(data);
            }
        }
    }

    return success;
}

}
