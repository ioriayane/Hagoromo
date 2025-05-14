#ifndef COMATPROTOREPOGETRECORDEX_H
#define COMATPROTOREPOGETRECORDEX_H

#include "atprotocol/com/atproto/repo/comatprotorepogetrecord.h"

namespace AtProtocolInterface {

class ComAtprotoRepoGetRecordEx : public ComAtprotoRepoGetRecord
{
public:
    explicit ComAtprotoRepoGetRecordEx(QObject *parent = nullptr);

    void profile(const QString &did);
    void list(const QString &did, const QString &rkey);
    void postGate(const QString &did, const QString &rkey);
    void skyBlurPost(const QString &did, const QString &rkey);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // COMATPROTOREPOGETRECORDEX_H
