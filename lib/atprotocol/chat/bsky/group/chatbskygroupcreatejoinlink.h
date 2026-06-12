#ifndef CHATBSKYGROUPCREATEJOINLINK_H
#define CHATBSKYGROUPCREATEJOINLINK_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupCreateJoinLink : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupCreateJoinLink(QObject *parent = nullptr);

    void createJoinLink(const QString &convoId, const bool requireApproval,
                        const QString &joinRule);

    const AtProtocolType::ChatBskyGroupDefs::JoinLinkView &joinLink() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyGroupDefs::JoinLinkView m_joinLink;
};

}

#endif // CHATBSKYGROUPCREATEJOINLINK_H
