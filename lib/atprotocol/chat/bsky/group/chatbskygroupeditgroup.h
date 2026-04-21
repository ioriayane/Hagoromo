#ifndef CHATBSKYGROUPEDITGROUP_H
#define CHATBSKYGROUPEDITGROUP_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupEditGroup : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupEditGroup(QObject *parent = nullptr);

    void editGroup(const QString &convoId, const QString &name);

    const AtProtocolType::ChatBskyConvoDefs::ConvoView &convo() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyConvoDefs::ConvoView m_convo;
};

}

#endif // CHATBSKYGROUPEDITGROUP_H
