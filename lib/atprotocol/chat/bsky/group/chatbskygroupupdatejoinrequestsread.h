#ifndef CHATBSKYGROUPUPDATEJOINREQUESTSREAD_H
#define CHATBSKYGROUPUPDATEJOINREQUESTSREAD_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupUpdateJoinRequestsRead : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupUpdateJoinRequestsRead(QObject *parent = nullptr);

    void updateJoinRequestsRead(const QString &convoId);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // CHATBSKYGROUPUPDATEJOINREQUESTSREAD_H
