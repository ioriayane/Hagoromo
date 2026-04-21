#ifndef CHATBSKYGROUPENABLEJOINLINK_H
#define CHATBSKYGROUPENABLEJOINLINK_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupEnableJoinLink : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupEnableJoinLink(QObject *parent = nullptr);

    void enableJoinLink(const QString &convoId);

    const AtProtocolType::ChatBskyGroupDefs::JoinLinkView &joinLink() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyGroupDefs::JoinLinkView m_joinLink;
};

}

#endif // CHATBSKYGROUPENABLEJOINLINK_H
