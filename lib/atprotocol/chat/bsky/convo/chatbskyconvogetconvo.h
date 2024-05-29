#ifndef CHATBSKYCONVOGETCONVO_H
#define CHATBSKYCONVOGETCONVO_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoGetConvo : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoGetConvo(QObject *parent = nullptr);

    void getConvo(const QString &convoId);

    const AtProtocolType::ChatBskyConvoDefs::ConvoView &convo() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyConvoDefs::ConvoView m_convo;
};

}

#endif // CHATBSKYCONVOGETCONVO_H
