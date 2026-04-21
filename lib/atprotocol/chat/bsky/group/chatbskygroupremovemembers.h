#ifndef CHATBSKYGROUPREMOVEMEMBERS_H
#define CHATBSKYGROUPREMOVEMEMBERS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupRemoveMembers : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupRemoveMembers(QObject *parent = nullptr);

    void removeMembers(const QString &convoId, const QList<QString> &members);

    const AtProtocolType::ChatBskyConvoDefs::ConvoView &convo() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyConvoDefs::ConvoView m_convo;
};

}

#endif // CHATBSKYGROUPREMOVEMEMBERS_H
