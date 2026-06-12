#ifndef CHATBSKYCONVOLISTCONVOREQUESTS_H
#define CHATBSKYCONVOLISTCONVOREQUESTS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoListConvoRequests : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoListConvoRequests(QObject *parent = nullptr);

    void listConvoRequests(const int limit, const QString &cursor);

    const QList<AtProtocolType::ChatBskyConvoDefs::ConvoView> &requestsConvoViewList() const;
    const QList<AtProtocolType::ChatBskyGroupDefs::JoinRequestConvoView> &
    requestsJoinRequestConvoViewList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::ChatBskyConvoDefs::ConvoView> m_requestsConvoViewList;
    QList<AtProtocolType::ChatBskyGroupDefs::JoinRequestConvoView>
            m_requestsJoinRequestConvoViewList;
};

}

#endif // CHATBSKYCONVOLISTCONVOREQUESTS_H
