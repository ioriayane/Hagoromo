#ifndef CHATBSKYCONVOUNLOCKCONVO_H
#define CHATBSKYCONVOUNLOCKCONVO_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoUnlockConvo : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoUnlockConvo(QObject *parent = nullptr);

    void unlockConvo(const QString &convoId);

    const AtProtocolType::ChatBskyConvoDefs::ConvoView &convo() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyConvoDefs::ConvoView m_convo;
};

}

#endif // CHATBSKYCONVOUNLOCKCONVO_H
