#ifndef CHATBSKYMODERATIONUPDATEACTORACCESS_H
#define CHATBSKYMODERATIONUPDATEACTORACCESS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyModerationUpdateActorAccess : public AccessAtProtocol
{
public:
    explicit ChatBskyModerationUpdateActorAccess(QObject *parent = nullptr);

    void updateActorAccess(const QString &actor, const bool allowAccess, const QString &ref);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // CHATBSKYMODERATIONUPDATEACTORACCESS_H
