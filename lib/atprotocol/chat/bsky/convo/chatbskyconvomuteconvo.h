#ifndef CHATBSKYCONVOMUTECONVO_H
#define CHATBSKYCONVOMUTECONVO_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoMuteConvo : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoMuteConvo(QObject *parent = nullptr);

    void muteConvo(const QString &convoId);

    const AtProtocolType::ChatBskyConvoDefs::ConvoView &convo() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyConvoDefs::ConvoView m_convo;
};

}

#endif // CHATBSKYCONVOMUTECONVO_H
