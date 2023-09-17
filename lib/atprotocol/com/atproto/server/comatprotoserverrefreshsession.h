#ifndef COMATPROTOSERVERREFRESHSESSION_H
#define COMATPROTOSERVERREFRESHSESSION_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoServerRefreshSession : public AccessAtProtocol
{
public:
    explicit ComAtprotoServerRefreshSession(QObject *parent = nullptr);

    void refreshSession();

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // COMATPROTOSERVERREFRESHSESSION_H
