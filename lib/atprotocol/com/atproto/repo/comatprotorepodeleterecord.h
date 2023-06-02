#ifndef COMATPROTOREPODELETERECORD_H
#define COMATPROTOREPODELETERECORD_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoRepoDeleteRecord : public AccessAtProtocol
{
public:
    explicit ComAtprotoRepoDeleteRecord(QObject *parent = nullptr);

    void deleteRecord(const QString &repo, const QString &collection, const QString &rkey,
                      const QString &swapRecord, const QString &swapCommit);

    void deleteLike(const QString &rkey);
    void deleteRepost(const QString &rkey);
    void unfollow(const QString &rkey);

private:
    virtual void parseJson(const QString reply_json);
};

}

#endif // COMATPROTOREPODELETERECORD_H
