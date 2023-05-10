#ifndef COMATPROTOREPOCREATERECORD_H
#define COMATPROTOREPOCREATERECORD_H

#include "accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoRepoCreateRecord : public AccessAtProtocol
{
public:
    explicit ComAtprotoRepoCreateRecord(QObject *parent = nullptr);

    void post(const QString &text);
    void like(const QString &cid, const QString &uri);

private:
    virtual void parseJson(const QString reply_json);
};

}

#endif // COMATPROTOREPOCREATERECORD_H
