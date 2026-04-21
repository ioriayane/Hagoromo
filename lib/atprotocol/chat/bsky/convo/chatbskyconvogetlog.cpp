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

const QList<AtProtocolType::ChatBskyConvoDefs::LogReadConvo> &
ChatBskyConvoGetLog::logsLogReadConvoList() const
{
    return m_logsLogReadConvoList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogAddMember> &
ChatBskyConvoGetLog::logsLogAddMemberList() const
{
    return m_logsLogAddMemberList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogRemoveMember> &
ChatBskyConvoGetLog::logsLogRemoveMemberList() const
{
    return m_logsLogRemoveMemberList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogMemberJoin> &
ChatBskyConvoGetLog::logsLogMemberJoinList() const
{
    return m_logsLogMemberJoinList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogMemberLeave> &
ChatBskyConvoGetLog::logsLogMemberLeaveList() const
{
    return m_logsLogMemberLeaveList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogLockConvo> &
ChatBskyConvoGetLog::logsLogLockConvoList() const
{
    return m_logsLogLockConvoList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogUnlockConvo> &
ChatBskyConvoGetLog::logsLogUnlockConvoList() const
{
    return m_logsLogUnlockConvoList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogLockConvoPermanently> &
ChatBskyConvoGetLog::logsLogLockConvoPermanentlyList() const
{
    return m_logsLogLockConvoPermanentlyList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogEditGroup> &
ChatBskyConvoGetLog::logsLogEditGroupList() const
{
    return m_logsLogEditGroupList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogCreateJoinLink> &
ChatBskyConvoGetLog::logsLogCreateJoinLinkList() const
{
    return m_logsLogCreateJoinLinkList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogEditJoinLink> &
ChatBskyConvoGetLog::logsLogEditJoinLinkList() const
{
    return m_logsLogEditJoinLinkList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogEnableJoinLink> &
ChatBskyConvoGetLog::logsLogEnableJoinLinkList() const
{
    return m_logsLogEnableJoinLinkList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogDisableJoinLink> &
ChatBskyConvoGetLog::logsLogDisableJoinLinkList() const
{
    return m_logsLogDisableJoinLinkList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogIncomingJoinRequest> &
ChatBskyConvoGetLog::logsLogIncomingJoinRequestList() const
{
    return m_logsLogIncomingJoinRequestList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogApproveJoinRequest> &
ChatBskyConvoGetLog::logsLogApproveJoinRequestList() const
{
    return m_logsLogApproveJoinRequestList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogRejectJoinRequest> &
ChatBskyConvoGetLog::logsLogRejectJoinRequestList() const
{
    return m_logsLogRejectJoinRequestList;
}

const QList<AtProtocolType::ChatBskyConvoDefs::LogOutgoingJoinRequest> &
ChatBskyConvoGetLog::logsLogOutgoingJoinRequestList() const
{
    return m_logsLogOutgoingJoinRequestList;
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
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logReadConvo")) {
                AtProtocolType::ChatBskyConvoDefs::LogReadConvo data;
                AtProtocolType::ChatBskyConvoDefs::copyLogReadConvo(value.toObject(), data);
                m_logsLogReadConvoList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logAddMember")) {
                AtProtocolType::ChatBskyConvoDefs::LogAddMember data;
                AtProtocolType::ChatBskyConvoDefs::copyLogAddMember(value.toObject(), data);
                m_logsLogAddMemberList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logRemoveMember")) {
                AtProtocolType::ChatBskyConvoDefs::LogRemoveMember data;
                AtProtocolType::ChatBskyConvoDefs::copyLogRemoveMember(value.toObject(), data);
                m_logsLogRemoveMemberList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logMemberJoin")) {
                AtProtocolType::ChatBskyConvoDefs::LogMemberJoin data;
                AtProtocolType::ChatBskyConvoDefs::copyLogMemberJoin(value.toObject(), data);
                m_logsLogMemberJoinList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logMemberLeave")) {
                AtProtocolType::ChatBskyConvoDefs::LogMemberLeave data;
                AtProtocolType::ChatBskyConvoDefs::copyLogMemberLeave(value.toObject(), data);
                m_logsLogMemberLeaveList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logLockConvo")) {
                AtProtocolType::ChatBskyConvoDefs::LogLockConvo data;
                AtProtocolType::ChatBskyConvoDefs::copyLogLockConvo(value.toObject(), data);
                m_logsLogLockConvoList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logUnlockConvo")) {
                AtProtocolType::ChatBskyConvoDefs::LogUnlockConvo data;
                AtProtocolType::ChatBskyConvoDefs::copyLogUnlockConvo(value.toObject(), data);
                m_logsLogUnlockConvoList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logLockConvoPermanently")) {
                AtProtocolType::ChatBskyConvoDefs::LogLockConvoPermanently data;
                AtProtocolType::ChatBskyConvoDefs::copyLogLockConvoPermanently(value.toObject(),
                                                                               data);
                m_logsLogLockConvoPermanentlyList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logEditGroup")) {
                AtProtocolType::ChatBskyConvoDefs::LogEditGroup data;
                AtProtocolType::ChatBskyConvoDefs::copyLogEditGroup(value.toObject(), data);
                m_logsLogEditGroupList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logCreateJoinLink")) {
                AtProtocolType::ChatBskyConvoDefs::LogCreateJoinLink data;
                AtProtocolType::ChatBskyConvoDefs::copyLogCreateJoinLink(value.toObject(), data);
                m_logsLogCreateJoinLinkList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logEditJoinLink")) {
                AtProtocolType::ChatBskyConvoDefs::LogEditJoinLink data;
                AtProtocolType::ChatBskyConvoDefs::copyLogEditJoinLink(value.toObject(), data);
                m_logsLogEditJoinLinkList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logEnableJoinLink")) {
                AtProtocolType::ChatBskyConvoDefs::LogEnableJoinLink data;
                AtProtocolType::ChatBskyConvoDefs::copyLogEnableJoinLink(value.toObject(), data);
                m_logsLogEnableJoinLinkList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logDisableJoinLink")) {
                AtProtocolType::ChatBskyConvoDefs::LogDisableJoinLink data;
                AtProtocolType::ChatBskyConvoDefs::copyLogDisableJoinLink(value.toObject(), data);
                m_logsLogDisableJoinLinkList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logIncomingJoinRequest")) {
                AtProtocolType::ChatBskyConvoDefs::LogIncomingJoinRequest data;
                AtProtocolType::ChatBskyConvoDefs::copyLogIncomingJoinRequest(value.toObject(),
                                                                              data);
                m_logsLogIncomingJoinRequestList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logApproveJoinRequest")) {
                AtProtocolType::ChatBskyConvoDefs::LogApproveJoinRequest data;
                AtProtocolType::ChatBskyConvoDefs::copyLogApproveJoinRequest(value.toObject(),
                                                                             data);
                m_logsLogApproveJoinRequestList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logRejectJoinRequest")) {
                AtProtocolType::ChatBskyConvoDefs::LogRejectJoinRequest data;
                AtProtocolType::ChatBskyConvoDefs::copyLogRejectJoinRequest(value.toObject(), data);
                m_logsLogRejectJoinRequestList.append(data);
            } else if (type == QStringLiteral("chat.bsky.convo.defs#logOutgoingJoinRequest")) {
                AtProtocolType::ChatBskyConvoDefs::LogOutgoingJoinRequest data;
                AtProtocolType::ChatBskyConvoDefs::copyLogOutgoingJoinRequest(value.toObject(),
                                                                              data);
                m_logsLogOutgoingJoinRequestList.append(data);
            }
        }
    }

    return success;
}

}
