#ifndef CHATBSKYCONVOUNMUTECONVO_H
#define CHATBSKYCONVOUNMUTECONVO_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoUnmuteConvo : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoUnmuteConvo(QObject *parent = nullptr);

    void unmuteConvo(const QString &convoId);

    const AtProtocolType::ChatBskyConvoDefs::ConvoView &convo() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyConvoDefs::ConvoView m_convo;
};

}

#endif // CHATBSKYCONVOUNMUTECONVO_H
