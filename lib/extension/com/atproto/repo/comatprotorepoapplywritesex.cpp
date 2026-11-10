#include "comatprotorepoapplywritesex.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonObject>

namespace AtProtocolInterface {

ComAtprotoRepoApplyWritesEx::ComAtprotoRepoApplyWritesEx(QObject *parent)
    : ComAtprotoRepoApplyWrites { parent }, m_createCount(0)
{
}

void ComAtprotoRepoApplyWritesEx::appendCreate(const QString &collection, const QString &rkey,
                                               const QJsonObject &record)
{
    QJsonObject json_value = record;
    json_value.insert(QStringLiteral("$type"), collection);

    QJsonObject json_write;
    json_write.insert(QStringLiteral("$type"),
                      QStringLiteral("com.atproto.repo.applyWrites#create"));
    json_write.insert(QStringLiteral("collection"), collection);
    json_write.insert(QStringLiteral("rkey"), rkey);
    json_write.insert(QStringLiteral("value"), json_value);
    m_writes.append(json_write);
    m_createCount++;
}

void ComAtprotoRepoApplyWritesEx::appendPost(const QString &rkey, const QString &text,
                                             const PostRecordData &data)
{
    appendCreate(QStringLiteral("app.bsky.feed.post"), rkey, makePostJsonObject(text, data));
}

void ComAtprotoRepoApplyWritesEx::appendThreadGate(
        const QString &uri, const AtProtocolType::ThreadGateType type,
        const QList<AtProtocolType::ThreadGateAllow> &allow_rules)
{
    appendCreate(QStringLiteral("app.bsky.feed.threadgate"),
                 AtProtocolType::LexiconsTypeUnknown::extractRkey(uri),
                 makeThreadGateJsonObject(uri, type, allow_rules));
}

void ComAtprotoRepoApplyWritesEx::appendPostGate(
        const QString &uri, const AtProtocolType::AppBskyFeedPostgate::MainEmbeddingRulesType type,
        const QStringList &detached_uris)
{
    appendCreate(QStringLiteral("app.bsky.feed.postgate"),
                 AtProtocolType::LexiconsTypeUnknown::extractRkey(uri),
                 makePostGateJsonObject(uri, type, detached_uris));
}

void ComAtprotoRepoApplyWritesEx::apply()
{
    if (m_writes.isEmpty()) {
        emit finished(false);
        return;
    }
    applyWrites(did(), true, m_writes, QString());
}

bool ComAtprotoRepoApplyWritesEx::parseJson(bool success, const QString reply_json)
{
    success = ComAtprotoRepoApplyWrites::parseJson(success, reply_json);
    if (success) {
        if (resultsCreateResultList().count() != m_createCount) {
            success = false;
        }
        for (const auto &result : resultsCreateResultList()) {
            if (result.uri.isEmpty() || result.cid.isEmpty()) {
                success = false;
            }
        }
    }
    return success;
}

}
