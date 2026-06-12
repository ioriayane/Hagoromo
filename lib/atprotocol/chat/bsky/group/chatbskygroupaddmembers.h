#ifndef CHATBSKYGROUPADDMEMBERS_H
#define CHATBSKYGROUPADDMEMBERS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupAddMembers : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupAddMembers(QObject *parent = nullptr);

    void addMembers(const QString &convoId, const QList<QString> &members);

    const AtProtocolType::ChatBskyConvoDefs::ConvoView &convo() const;
    const QList<AtProtocolType::ChatBskyActorDefs::ProfileViewBasic> &addedMembersList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyConvoDefs::ConvoView m_convo;
    QList<AtProtocolType::ChatBskyActorDefs::ProfileViewBasic> m_addedMembersList;
};

}

#endif // CHATBSKYGROUPADDMEMBERS_H
