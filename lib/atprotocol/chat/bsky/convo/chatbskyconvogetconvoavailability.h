#ifndef CHATBSKYCONVOGETCONVOAVAILABILITY_H
#define CHATBSKYCONVOGETCONVOAVAILABILITY_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoGetConvoAvailability : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoGetConvoAvailability(QObject *parent = nullptr);

    void getConvoAvailability(const QList<QString> &members);

    const bool &canChat() const;
    const AtProtocolType::ChatBskyConvoDefs::ConvoView &convo() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    bool m_canChat;
    AtProtocolType::ChatBskyConvoDefs::ConvoView m_convo;
};

}

#endif // CHATBSKYCONVOGETCONVOAVAILABILITY_H
