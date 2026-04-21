#ifndef CHATBSKYCONVOGETLOG_H
#define CHATBSKYCONVOGETLOG_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoGetLog : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoGetLog(QObject *parent = nullptr);

    void getLog(const QString &cursor);

    const QList<AtProtocolType::ChatBskyConvoDefs::LogBeginConvo> &logsLogBeginConvoList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogAcceptConvo> &logsLogAcceptConvoList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogLeaveConvo> &logsLogLeaveConvoList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogMuteConvo> &logsLogMuteConvoList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogUnmuteConvo> &logsLogUnmuteConvoList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogCreateMessage> &
    logsLogCreateMessageList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogDeleteMessage> &
    logsLogDeleteMessageList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogReadMessage> &logsLogReadMessageList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogAddReaction> &logsLogAddReactionList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogRemoveReaction> &
    logsLogRemoveReactionList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogReadConvo> &logsLogReadConvoList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogAddMember> &logsLogAddMemberList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogRemoveMember> &
    logsLogRemoveMemberList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogMemberJoin> &logsLogMemberJoinList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogMemberLeave> &logsLogMemberLeaveList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogLockConvo> &logsLogLockConvoList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogUnlockConvo> &logsLogUnlockConvoList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogLockConvoPermanently> &
    logsLogLockConvoPermanentlyList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogEditGroup> &logsLogEditGroupList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogCreateJoinLink> &
    logsLogCreateJoinLinkList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogEditJoinLink> &
    logsLogEditJoinLinkList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogEnableJoinLink> &
    logsLogEnableJoinLinkList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogDisableJoinLink> &
    logsLogDisableJoinLinkList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogIncomingJoinRequest> &
    logsLogIncomingJoinRequestList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogApproveJoinRequest> &
    logsLogApproveJoinRequestList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogRejectJoinRequest> &
    logsLogRejectJoinRequestList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::LogOutgoingJoinRequest> &
    logsLogOutgoingJoinRequestList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::ChatBskyConvoDefs::LogBeginConvo> m_logsLogBeginConvoList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogAcceptConvo> m_logsLogAcceptConvoList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogLeaveConvo> m_logsLogLeaveConvoList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogMuteConvo> m_logsLogMuteConvoList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogUnmuteConvo> m_logsLogUnmuteConvoList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogCreateMessage> m_logsLogCreateMessageList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogDeleteMessage> m_logsLogDeleteMessageList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogReadMessage> m_logsLogReadMessageList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogAddReaction> m_logsLogAddReactionList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogRemoveReaction> m_logsLogRemoveReactionList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogReadConvo> m_logsLogReadConvoList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogAddMember> m_logsLogAddMemberList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogRemoveMember> m_logsLogRemoveMemberList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogMemberJoin> m_logsLogMemberJoinList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogMemberLeave> m_logsLogMemberLeaveList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogLockConvo> m_logsLogLockConvoList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogUnlockConvo> m_logsLogUnlockConvoList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogLockConvoPermanently>
            m_logsLogLockConvoPermanentlyList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogEditGroup> m_logsLogEditGroupList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogCreateJoinLink> m_logsLogCreateJoinLinkList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogEditJoinLink> m_logsLogEditJoinLinkList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogEnableJoinLink> m_logsLogEnableJoinLinkList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogDisableJoinLink> m_logsLogDisableJoinLinkList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogIncomingJoinRequest>
            m_logsLogIncomingJoinRequestList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogApproveJoinRequest> m_logsLogApproveJoinRequestList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogRejectJoinRequest> m_logsLogRejectJoinRequestList;
    QList<AtProtocolType::ChatBskyConvoDefs::LogOutgoingJoinRequest>
            m_logsLogOutgoingJoinRequestList;
};

}

#endif // CHATBSKYCONVOGETLOG_H
