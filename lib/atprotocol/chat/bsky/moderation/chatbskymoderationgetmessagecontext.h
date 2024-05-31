#ifndef CHATBSKYMODERATIONGETMESSAGECONTEXT_H
#define CHATBSKYMODERATIONGETMESSAGECONTEXT_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyModerationGetMessageContext : public AccessAtProtocol
{
public:
    explicit ChatBskyModerationGetMessageContext(QObject *parent = nullptr);

    void getMessageContext(const QString &messageId, const int before, const int after);

    const QList<AtProtocolType::ChatBskyConvoDefs::MessageView> &messagesMessageViewList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::DeletedMessageView> &
    messagesDeletedMessageViewList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::ChatBskyConvoDefs::MessageView> m_messagesMessageViewList;
    QList<AtProtocolType::ChatBskyConvoDefs::DeletedMessageView> m_messagesDeletedMessageViewList;
};

}

#endif // CHATBSKYMODERATIONGETMESSAGECONTEXT_H
