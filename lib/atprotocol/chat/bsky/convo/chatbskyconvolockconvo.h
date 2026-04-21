#ifndef CHATBSKYCONVOLOCKCONVO_H
#define CHATBSKYCONVOLOCKCONVO_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoLockConvo : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoLockConvo(QObject *parent = nullptr);

    void lockConvo(const QString &convoId);

    const AtProtocolType::ChatBskyConvoDefs::ConvoView &convo() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyConvoDefs::ConvoView m_convo;
};

}

#endif // CHATBSKYCONVOLOCKCONVO_H
