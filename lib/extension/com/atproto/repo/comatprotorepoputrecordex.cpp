#include "comatprotorepoputrecordex.h"

namespace AtProtocolInterface {

ComAtprotoRepoPutRecordEx::ComAtprotoRepoPutRecordEx(QObject *parent)
    : ComAtprotoRepoPutRecord { parent }
{
}

void ComAtprotoRepoPutRecordEx::profile(const AtProtocolType::Blob &avatar,
                                        const AtProtocolType::Blob &banner,
                                        const QString &description, const QString &display_name,
                                        const QString &cid)
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

bool ComAtprotoRepoPutRecordEx::parseJson(bool success, const QString reply_json)
{
    success = ComAtprotoRepoPutRecord::parseJson(success, reply_json);

    return success;
}

}
