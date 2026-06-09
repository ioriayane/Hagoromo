#ifndef CHATBSKYCONVOGETCONVOMEMBERS_H
#define CHATBSKYCONVOGETCONVOMEMBERS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoGetConvoMembers : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoGetConvoMembers(QObject *parent = nullptr);

    void getConvoMembers(const QString &convoId, const int limit, const QString &cursor);

    const QList<AtProtocolType::ChatBskyActorDefs::ProfileViewBasic> &membersList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::ChatBskyActorDefs::ProfileViewBasic> m_membersList;
};

}

#endif // CHATBSKYCONVOGETCONVOMEMBERS_H
