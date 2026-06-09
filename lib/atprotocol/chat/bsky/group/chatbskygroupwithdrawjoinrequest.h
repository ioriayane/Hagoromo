#ifndef CHATBSKYGROUPWITHDRAWJOINREQUEST_H
#define CHATBSKYGROUPWITHDRAWJOINREQUEST_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupWithdrawJoinRequest : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupWithdrawJoinRequest(QObject *parent = nullptr);

    void withdrawJoinRequest(const QString &convoId);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // CHATBSKYGROUPWITHDRAWJOINREQUEST_H
