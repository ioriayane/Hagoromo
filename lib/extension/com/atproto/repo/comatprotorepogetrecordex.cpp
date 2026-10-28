#include "comatprotorepogetrecordex.h"

#include <QJsonDocument>

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
    getRecord(did, QStringLiteral("app.bsky.notification.declaration"), QStringLiteral("self"),
              QString());
}

void ComAtprotoRepoGetRecordEx::profileLink(const QString &did, const QString &rkey)
{
    getRecord(did, QStringLiteral("app.bsky.actor.link"), rkey, QString());
}

const QJsonObject &ComAtprotoRepoGetRecordEx::rawValue() const
{
    return m_rawValue;
}

bool ComAtprotoRepoGetRecordEx::parseJson(bool success, const QString reply_json)
{
    success = ComAtprotoRepoGetRecord::parseJson(success, reply_json);
    if (success) {
        m_rawValue =
                QJsonDocument::fromJson(reply_json.toUtf8()).object().value("value").toObject();
    }

    return success;
}

}
