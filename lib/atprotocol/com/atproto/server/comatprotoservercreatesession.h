#ifndef COMATPROTOSERVERCREATESESSION_H
#define COMATPROTOSERVERCREATESESSION_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoServerCreateSession : public AccessAtProtocol
{
public:
    explicit ComAtprotoServerCreateSession(QObject *parent = nullptr);
    ~ComAtprotoServerCreateSession();

    void create(const QString &id, const QString &password);

private:
    virtual void parseJson(bool success, const QString reply_json);
};

}

#endif // COMATPROTOSERVERCREATESESSION_H
