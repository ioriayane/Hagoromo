#ifndef COMATPROTOSERVERCREATESESSION_H
#define COMATPROTOSERVERCREATESESSION_H

#include "accessatprotocol.h"

class ComAtprotoServerCreateSession : public AccessAtProtocol
{
public:
    explicit ComAtprotoServerCreateSession(QObject *parent = nullptr);

    void create(const QString &id, const QString &password);

private:
    virtual void parseJson(const QString reply_json);
};

#endif // COMATPROTOSERVERCREATESESSION_H
