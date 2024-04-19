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

bool ComAtprotoRepoGetRecordEx::parseJson(bool success, const QString reply_json)
{
    success = ComAtprotoRepoGetRecord::parseJson(success, reply_json);

    return success;
}

}
