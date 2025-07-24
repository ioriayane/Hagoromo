#include "comatprotorepogetrecordex.h"

namespace AtProtocolInterface {

ComAtprotoRepoGetRecordEx::ComAtprotoRepoGetRecordEx(QObject *parent)
    : ComAtprotoRepoGetRecord { parent }
{
}

void ComAtprotoRepoGetRecordEx::profile(const QString &did)
{
    getRecord(did, QStringLiteral("app.bsky.actor.profile"), QStringLiteral("self"), QString());
}

void ComAtprotoRepoGetRecordEx::list(const QString &did, const QString &rkey)
{
    getRecord(did, QStringLiteral("app.bsky.graph.list"), rkey, QString());
}

void ComAtprotoRepoGetRecordEx::postGate(const QString &did, const QString &rkey)
{
    getRecord(did, QStringLiteral("app.bsky.feed.postgate"), rkey, QString());
}

void ComAtprotoRepoGetRecordEx::skyBlurPost(const QString &did, const QString &rkey)
{
    getRecord(did, QStringLiteral("uk.skyblur.post"), rkey, QString());
}

void ComAtprotoRepoGetRecordEx::notificationDeclaration(const QString &did)
{
    getRecord(did, QStringLiteral("app.bsky.notification.declaration"), QStringLiteral("self"), QString());
}

bool ComAtprotoRepoGetRecordEx::parseJson(bool success, const QString reply_json)
{
    success = ComAtprotoRepoGetRecord::parseJson(success, reply_json);

    return success;
}

}
