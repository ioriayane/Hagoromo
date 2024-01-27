#include "comatprotorepodeleterecord.h"

#include <QJsonDocument>
#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ComAtprotoRepoDeleteRecord::ComAtprotoRepoDeleteRecord(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ComAtprotoRepoDeleteRecord::deleteRecord(const QString &repo, const QString &collection,
                                              const QString &rkey, const QString &swapRecord,
                                              const QString &swapCommit)
{
    QJsonObject json_obj;
    json_obj.insert("repo", repo);
    json_obj.insert("collection", collection);
    json_obj.insert("rkey", rkey);
    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/com.atproto.repo.deleteRecord"),
         json_doc.toJson(QJsonDocument::Compact));
}

void ComAtprotoRepoDeleteRecord::deletePost(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.feed.post"), rkey, QString(), QString());
}

void ComAtprotoRepoDeleteRecord::deleteLike(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.feed.like"), rkey, QString(), QString());
}

void ComAtprotoRepoDeleteRecord::deleteRepost(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.feed.repost"), rkey, QString(), QString());
}

void ComAtprotoRepoDeleteRecord::unfollow(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.graph.follow"), rkey, QString(), QString());
}

void ComAtprotoRepoDeleteRecord::deleteBlock(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.graph.block"), rkey, QString(), QString());
}

void ComAtprotoRepoDeleteRecord::deleteList(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.graph.list"), rkey, QString(), QString());
}

void ComAtprotoRepoDeleteRecord::deleteListItem(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.graph.listitem"), rkey, QString(),
                 QString());
}

bool ComAtprotoRepoDeleteRecord::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
