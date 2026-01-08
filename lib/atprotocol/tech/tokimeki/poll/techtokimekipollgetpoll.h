#ifndef TECHTOKIMEKIPOLLGETPOLL_H
#define TECHTOKIMEKIPOLLGETPOLL_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class TechTokimekiPollGetPoll : public AccessAtProtocol
{
public:
    explicit TechTokimekiPollGetPoll(QObject *parent = nullptr);

    void getPoll(const QString &uri, const QString &viewer);

    const AtProtocolType::TechTokimekiPollDefs::PollViewDetailed &pollViewDetailed() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::TechTokimekiPollDefs::PollViewDetailed m_pollViewDetailed;
};

}

#endif // TECHTOKIMEKIPOLLGETPOLL_H
