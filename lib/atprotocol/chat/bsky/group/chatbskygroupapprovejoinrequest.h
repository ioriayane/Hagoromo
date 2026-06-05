#ifndef CHATBSKYGROUPAPPROVEJOINREQUEST_H
#define CHATBSKYGROUPAPPROVEJOINREQUEST_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupApproveJoinRequest : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupApproveJoinRequest(QObject *parent = nullptr);

    void approveJoinRequest(const QString &convoId, const QString &member);

    const AtProtocolType::ChatBskyConvoDefs::ConvoView &convo() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyConvoDefs::ConvoView m_convo;
};

}

#endif // CHATBSKYGROUPAPPROVEJOINREQUEST_H
