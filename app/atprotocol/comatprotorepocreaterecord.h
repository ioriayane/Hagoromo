#ifndef COMATPROTOREPOCREATERECORD_H
#define COMATPROTOREPOCREATERECORD_H

#include "accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoRepoCreateRecord : public AccessAtProtocol
{
public:
    explicit ComAtprotoRepoCreateRecord(QObject *parent = nullptr);

    void createRecord(const QString &text);

private:
    virtual void parseJson(const QString reply_json);
};

}

#endif // COMATPROTOREPOCREATERECORD_H
