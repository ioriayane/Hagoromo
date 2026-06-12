#ifndef CHATBSKYACTORGETSTATUS_H
#define CHATBSKYACTORGETSTATUS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyActorGetStatus : public AccessAtProtocol
{
public:
    explicit ChatBskyActorGetStatus(QObject *parent = nullptr);

    void getStatus();

    const bool &chatDisabled() const;
    const bool &canCreateGroups() const;
    const int &groupMemberLimit() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    bool m_chatDisabled;
    bool m_canCreateGroups;
    int m_groupMemberLimit;
};

}

#endif // CHATBSKYACTORGETSTATUS_H
