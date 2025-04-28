#ifndef CHATBSKYCONVOADDREACTION_H
#define CHATBSKYCONVOADDREACTION_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoAddReaction : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoAddReaction(QObject *parent = nullptr);

    void addReaction(const QString &convoId, const QString &messageId, const QString &value);

    const AtProtocolType::ChatBskyConvoDefs::MessageView &message() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyConvoDefs::MessageView m_message;
};

}

#endif // CHATBSKYCONVOADDREACTION_H
