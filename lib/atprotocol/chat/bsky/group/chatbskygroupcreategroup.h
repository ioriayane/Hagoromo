#ifndef CHATBSKYGROUPCREATEGROUP_H
#define CHATBSKYGROUPCREATEGROUP_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupCreateGroup : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupCreateGroup(QObject *parent = nullptr);

    void createGroup(const QList<QString> &members, const QString &name);

    const AtProtocolType::ChatBskyConvoDefs::ConvoView &convo() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyConvoDefs::ConvoView m_convo;
};

}

#endif // CHATBSKYGROUPCREATEGROUP_H
