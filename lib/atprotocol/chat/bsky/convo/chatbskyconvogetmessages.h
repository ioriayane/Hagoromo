#ifndef CHATBSKYCONVOGETMESSAGES_H
#define CHATBSKYCONVOGETMESSAGES_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoGetMessages : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoGetMessages(QObject *parent = nullptr);

    void getMessages(const QString &convoId, const int limit, const QString &cursor);

    const QList<AtProtocolType::ChatBskyConvoDefs::MessageView> &messagesMessageViewList() const;
    const QList<AtProtocolType::ChatBskyConvoDefs::DeletedMessageView> &
    messagesDeletedMessageViewList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::ChatBskyConvoDefs::MessageView> m_messagesMessageViewList;
    QList<AtProtocolType::ChatBskyConvoDefs::DeletedMessageView> m_messagesDeletedMessageViewList;
};

}

#endif // CHATBSKYCONVOGETMESSAGES_H
