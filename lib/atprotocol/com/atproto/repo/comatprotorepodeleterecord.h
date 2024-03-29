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

    void deletePost(const QString &rkey);
    void deleteLike(const QString &rkey);
    void deleteRepost(const QString &rkey);
    void unfollow(const QString &rkey);
    void deleteBlock(const QString &rkey);
    void deleteList(const QString &rkey);
    void deleteListItem(const QString &rkey);
    void deleteThreadGate(const QString &rkey);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // COMATPROTOREPODELETERECORD_H
