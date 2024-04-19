#include "comatprotorepodeleterecordex.h"

namespace AtProtocolInterface {

ComAtprotoRepoDeleteRecordEx::ComAtprotoRepoDeleteRecordEx(QObject *parent)
    : ComAtprotoRepoDeleteRecord { parent }
{
}

void ComAtprotoRepoDeleteRecordEx::deletePost(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.feed.post"), rkey, QString(), QString());
}

void ComAtprotoRepoDeleteRecordEx::deleteLike(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.feed.like"), rkey, QString(), QString());
}

void ComAtprotoRepoDeleteRecordEx::deleteRepost(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.feed.repost"), rkey, QString(), QString());
}

void ComAtprotoRepoDeleteRecordEx::unfollow(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.graph.follow"), rkey, QString(), QString());
}

void ComAtprotoRepoDeleteRecordEx::deleteBlock(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.graph.block"), rkey, QString(), QString());
}

void ComAtprotoRepoDeleteRecordEx::deleteBlockList(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.graph.listblock"), rkey, QString(),
                 QString());
}

void ComAtprotoRepoDeleteRecordEx::deleteList(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.graph.list"), rkey, QString(), QString());
}

void ComAtprotoRepoDeleteRecordEx::deleteListItem(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.graph.listitem"), rkey, QString(),
                 QString());
}

void ComAtprotoRepoDeleteRecordEx::deleteThreadGate(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.feed.threadgate"), rkey, QString(),
                 QString());
}

bool ComAtprotoRepoDeleteRecordEx::parseJson(bool success, const QString reply_json)
{
    success = ComAtprotoRepoDeleteRecord::parseJson(success, reply_json);

    return success;
}

}
