#ifndef CHATBSKYGROUPEDITJOINLINK_H
#define CHATBSKYGROUPEDITJOINLINK_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupEditJoinLink : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupEditJoinLink(QObject *parent = nullptr);

    void editJoinLink(const QString &convoId, const bool requireApproval, const QString &joinRule);

    const AtProtocolType::ChatBskyGroupDefs::JoinLinkView &joinLink() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyGroupDefs::JoinLinkView m_joinLink;
};

}

#endif // CHATBSKYGROUPEDITJOINLINK_H
