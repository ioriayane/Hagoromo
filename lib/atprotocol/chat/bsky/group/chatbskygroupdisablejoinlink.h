#ifndef CHATBSKYGROUPDISABLEJOINLINK_H
#define CHATBSKYGROUPDISABLEJOINLINK_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupDisableJoinLink : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupDisableJoinLink(QObject *parent = nullptr);

    void disableJoinLink(const QString &convoId);

    const AtProtocolType::ChatBskyGroupDefs::JoinLinkView &joinLink() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyGroupDefs::JoinLinkView m_joinLink;
};

}

#endif // CHATBSKYGROUPDISABLEJOINLINK_H
