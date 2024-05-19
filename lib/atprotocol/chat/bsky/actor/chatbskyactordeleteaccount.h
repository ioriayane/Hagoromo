#ifndef CHATBSKYACTORDELETEACCOUNT_H
#define CHATBSKYACTORDELETEACCOUNT_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyActorDeleteAccount : public AccessAtProtocol
{
public:
    explicit ChatBskyActorDeleteAccount(QObject *parent = nullptr);

    void deleteAccount();

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // CHATBSKYACTORDELETEACCOUNT_H
