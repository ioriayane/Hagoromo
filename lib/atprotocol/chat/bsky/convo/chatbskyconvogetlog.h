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
};

}

#endif // CHATBSKYCONVOGETLOG_H
