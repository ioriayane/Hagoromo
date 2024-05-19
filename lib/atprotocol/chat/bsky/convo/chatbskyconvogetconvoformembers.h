#ifndef CHATBSKYCONVOGETCONVOFORMEMBERS_H
#define CHATBSKYCONVOGETCONVOFORMEMBERS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoGetConvoForMembers : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoGetConvoForMembers(QObject *parent = nullptr);

    void getConvoForMembers(const QList<QString> &members);

    const AtProtocolType::ChatBskyConvoDefs::ConvoView &convo() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyConvoDefs::ConvoView m_convo;
};

}

#endif // CHATBSKYCONVOGETCONVOFORMEMBERS_H
