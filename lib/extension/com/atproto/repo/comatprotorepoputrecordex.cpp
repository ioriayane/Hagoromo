#include "comatprotorepoputrecordex.h"
#include "atprotocol/lexicons_func_unknown.h"

namespace AtProtocolInterface {

ComAtprotoRepoPutRecordEx::ComAtprotoRepoPutRecordEx(QObject *parent)
    : ComAtprotoRepoPutRecord { parent }
{
}

void ComAtprotoRepoPutRecordEx::profile(
        const AtProtocolType::Blob &avatar, const AtProtocolType::Blob &banner,
        const QString &description, const QString &display_name,
        const AtProtocolType::ComAtprotoRepoStrongRef::Main &pinned_post, const QString &cid)
{
    QString type = QStringLiteral("app.bsky.actor.profile");
    QJsonObject json_record;
    json_record.insert("$type", type);
    if (!description.isEmpty()) {
        json_record.insert("description", description);
    }
    if (!display_name.isEmpty()) {
        json_record.insert("displayName", display_name);
    }
    if (!avatar.cid.isEmpty()) {
        QJsonObject json_avatar;
        setJsonBlob(avatar, json_avatar);
        json_record.insert("avatar", json_avatar);
    }
    if (!banner.cid.isEmpty()) {
        QJsonObject json_banner;
        setJsonBlob(banner, json_banner);
        json_record.insert("banner", json_banner);
    }
    if (!pinned_post.uri.isEmpty()) {
        QJsonObject json_subject;
        json_subject.insert("cid", pinned_post.cid);
        json_subject.insert("uri", pinned_post.uri);
        json_record.insert("pinnedPost", json_subject);
    }

    putRecord(this->did(), type, QStringLiteral("self"), true, json_record, cid, QString());
}

void ComAtprotoRepoPutRecordEx::list(const AtProtocolType::Blob &avatar, const QString &purpose,
                                     const QString &description, const QString &name,
                                     const QString &rkey)
{
    QString type = QStringLiteral("app.bsky.graph.list");
    QJsonObject json_record;
    json_record.insert("$type", type);
    json_record.insert("purpose", purpose);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    if (!description.isEmpty()) {
        json_record.insert("description", description);
    }
    if (!name.isEmpty()) {
        json_record.insert("name", name);
    }
    if (!avatar.cid.isEmpty()) {
        QJsonObject json_avatar;
        setJsonBlob(avatar, json_avatar);
        json_record.insert("avatar", json_avatar);
    }

    putRecord(this->did(), type, rkey, true, json_record, QString(), QString());
}

void ComAtprotoRepoPutRecordEx::postGate(
        const QString &uri, const AtProtocolType::AppBskyFeedPostgate::MainEmbeddingRulesType type,
        const QStringList &detached_uris)
{
    if (!uri.startsWith("at://")) {
        emit finished(false);
        return;
    }
    // ruleの設定もdetach uriも空で送信したい
    // if (type == AtProtocolType::AppBskyFeedPostgate::MainEmbeddingRulesType::none
    //     && detached_uris.isEmpty()) {
    //     emit finished(false);
    //     return;
    // }

    QString rkey = AtProtocolType::LexiconsTypeUnknown::extractRkey(uri);

    putRecord(this->did(), QStringLiteral("app.bsky.feed.postgate"), rkey, true,
              makePostGateJsonObject(uri, type, detached_uris), QString(), QString());
}

bool ComAtprotoRepoPutRecordEx::parseJson(bool success, const QString reply_json)
{
    success = ComAtprotoRepoPutRecord::parseJson(success, reply_json);

    return success;
}

}
