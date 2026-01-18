#ifndef COMATPROTOSERVERREFRESHSESSIONEX_H
#define COMATPROTOSERVERREFRESHSESSIONEX_H

#include "atprotocol/com/atproto/server/comatprotoserverrefreshsession.h"

namespace AtProtocolInterface {

class ComAtprotoServerRefreshSessionEx : public ComAtprotoServerRefreshSession
{
public:
    explicit ComAtprotoServerRefreshSessionEx(QObject *parent = nullptr);

    void refreshSession();

    const QString &email() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // COMATPROTOSERVERREFRESHSESSIONEX_H
