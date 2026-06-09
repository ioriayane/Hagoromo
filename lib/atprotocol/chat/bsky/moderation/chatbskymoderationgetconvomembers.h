#ifndef CHATBSKYMODERATIONGETCONVOMEMBERS_H
#define CHATBSKYMODERATIONGETCONVOMEMBERS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyModerationGetConvoMembers : public AccessAtProtocol
{
public:
    explicit ChatBskyModerationGetConvoMembers(QObject *parent = nullptr);

    void getConvoMembers(const QString &convoId, const int limit, const QString &cursor);

    const QList<AtProtocolType::ChatBskyActorDefs::ProfileViewBasic> &membersList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::ChatBskyActorDefs::ProfileViewBasic> m_membersList;
};

}

#endif // CHATBSKYMODERATIONGETCONVOMEMBERS_H
