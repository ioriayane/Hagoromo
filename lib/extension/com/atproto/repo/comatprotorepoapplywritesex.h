#ifndef COMATPROTOREPOAPPLYWRITESEX_H
#define COMATPROTOREPOAPPLYWRITESEX_H

#include "atprotocol/com/atproto/repo/comatprotorepoapplywrites.h"

#include <QJsonArray>

namespace AtProtocolInterface {

class ComAtprotoRepoApplyWritesEx : public ComAtprotoRepoApplyWrites
{
public:
    explicit ComAtprotoRepoApplyWritesEx(QObject *parent = nullptr);

    // レコードは追加した順に書き込まれる
    void appendCreate(const QString &collection, const QString &rkey, const QJsonObject &record);
    void appendPost(const QString &rkey, const QString &text, const PostRecordData &data);
    // ゲートのレコードのrkeyは対象のポストと同じにする必要があるのでuriから取得する
    void appendThreadGate(const QString &uri, const AtProtocolType::ThreadGateType type,
                          const QList<AtProtocolType::ThreadGateAllow> &allow_rules);
    void appendPostGate(const QString &uri,
                        const AtProtocolType::AppBskyFeedPostgate::MainEmbeddingRulesType type,
                        const QStringList &detached_uris);
    void apply();

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QJsonArray m_writes;
    int m_createCount;
};

}

#endif // COMATPROTOREPOAPPLYWRITESEX_H
