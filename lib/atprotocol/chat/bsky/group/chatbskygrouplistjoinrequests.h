#ifndef CHATBSKYGROUPLISTJOINREQUESTS_H
#define CHATBSKYGROUPLISTJOINREQUESTS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupListJoinRequests : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupListJoinRequests(QObject *parent = nullptr);

    void listJoinRequests(const QString &convoId, const int limit, const QString &cursor);

    const QList<AtProtocolType::ChatBskyGroupDefs::JoinRequestView> &requestsList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::ChatBskyGroupDefs::JoinRequestView> m_requestsList;
};

}

#endif // CHATBSKYGROUPLISTJOINREQUESTS_H
