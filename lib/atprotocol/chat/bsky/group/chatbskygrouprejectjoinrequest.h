#ifndef CHATBSKYGROUPREJECTJOINREQUEST_H
#define CHATBSKYGROUPREJECTJOINREQUEST_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupRejectJoinRequest : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupRejectJoinRequest(QObject *parent = nullptr);

    void rejectJoinRequest(const QString &convoId, const QString &member);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // CHATBSKYGROUPREJECTJOINREQUEST_H
