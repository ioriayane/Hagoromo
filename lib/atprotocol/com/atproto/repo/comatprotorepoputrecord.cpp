#include "comatprotorepoputrecord.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ComAtprotoRepoPutRecord::ComAtprotoRepoPutRecord(QObject *parent) : AccessAtProtocol { parent } { }

bool ComAtprotoRepoPutRecord::putRecord(const QString &repo, const QString &collection,
                                        const QString &rkey, const bool validate,
                                        const QString &swapRecord, const QString &swapCommit,
                                        const QJsonObject &record)
{
    QJsonObject json_obj;
    json_obj.insert("repo", repo);
    json_obj.insert("collection", collection);
    if (!rkey.isEmpty()) {
        json_obj.insert("rkey", rkey);
    }
    json_obj.insert("validate", validate); // default = true
    if (!swapRecord.isEmpty()) {
        json_obj.insert("swapRecord", swapRecord);
    }
    if (!swapCommit.isEmpty()) {
        json_obj.insert("swapCommit", swapCommit);
    }
    json_obj.insert("record", record);

    QJsonDocument json_doc(json_obj);

    return post(QStringLiteral("xrpc/com.atproto.repo.putRecord"),
                json_doc.toJson(QJsonDocument::Compact), true);
}

bool ComAtprotoRepoPutRecord::profile(const AtProtocolType::Blob &avatar,
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

    return putRecord(this->did(), type, QStringLiteral("self"), true, cid, QString(), json_record);
}

bool ComAtprotoRepoPutRecord::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        // setCursor(json_doc.object().value("cursor").toString());
    }

    return success;
}

}
