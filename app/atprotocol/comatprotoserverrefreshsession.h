#ifndef COMATPROTOSERVERREFRESHSESSION_H
#define COMATPROTOSERVERREFRESHSESSION_H

#include "accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoServerRefreshSession : public AccessAtProtocol
{
public:
    explicit ComAtprotoServerRefreshSession(QObject *parent = nullptr);

    void refreshSession();

private:
    virtual void parseJson(const QString reply_json);
};

}

#endif // COMATPROTOSERVERREFRESHSESSION_H