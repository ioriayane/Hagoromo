#ifndef COMATPROTOSERVERCREATESESSIONEX_H
#define COMATPROTOSERVERCREATESESSIONEX_H

#include "atprotocol/com/atproto/server/comatprotoservercreatesession.h"

namespace AtProtocolInterface {

class ComAtprotoServerCreateSessionEx : public ComAtprotoServerCreateSession
{
public:
    explicit ComAtprotoServerCreateSessionEx(QObject *parent = nullptr);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // COMATPROTOSERVERCREATESESSIONEX_H
