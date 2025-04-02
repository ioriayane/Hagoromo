#ifndef CHATBSKYCONVOREMOVEREACTION_H
#define CHATBSKYCONVOREMOVEREACTION_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoRemoveReaction : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoRemoveReaction(QObject *parent = nullptr);

    void removeReaction(const QString &convoId, const QString &messageId, const QString &value);

    const AtProtocolType::ChatBskyConvoDefs::MessageView &message() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyConvoDefs::MessageView m_message;
};

}

#endif // CHATBSKYCONVOREMOVEREACTION_H
