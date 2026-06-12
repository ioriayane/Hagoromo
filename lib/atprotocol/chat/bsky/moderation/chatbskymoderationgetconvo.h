#ifndef CHATBSKYMODERATIONGETCONVO_H
#define CHATBSKYMODERATIONGETCONVO_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyModerationGetConvo : public AccessAtProtocol
{
public:
    explicit ChatBskyModerationGetConvo(QObject *parent = nullptr);

    void getConvo(const QString &convoId);

    const AtProtocolType::ChatBskyModerationDefs::ConvoView &convo() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyModerationDefs::ConvoView m_convo;
};

}

#endif // CHATBSKYMODERATIONGETCONVO_H
