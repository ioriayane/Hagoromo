#ifndef COMATPROTOREPODELETERECORDEX_H
#define COMATPROTOREPODELETERECORDEX_H

#include "atprotocol/com/atproto/repo/comatprotorepodeleterecord.h"

namespace AtProtocolInterface {

class ComAtprotoRepoDeleteRecordEx : public ComAtprotoRepoDeleteRecord
{
public:
    explicit ComAtprotoRepoDeleteRecordEx(QObject *parent = nullptr);

    void deletePost(const QString &rkey);
    void deleteLike(const QString &rkey);
    void deleteRepost(const QString &rkey);
    void unfollow(const QString &rkey);
    void deleteBlock(const QString &rkey);
    void deleteBlockList(const QString &rkey);
    void deleteList(const QString &rkey);
    void deleteListItem(const QString &rkey);
    void deleteThreadGate(const QString &rkey);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // COMATPROTOREPODELETERECORDEX_H
