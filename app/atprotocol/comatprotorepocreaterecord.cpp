#include "comatprotorepocreaterecord.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ComAtprotoRepoCreateRecord::ComAtprotoRepoCreateRecord(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ComAtprotoRepoCreateRecord::post(const QString &text)
{
    QJsonObject json_record;
    json_record.insert("text", text);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    QJsonObject json_obj;
    json_obj.insert("repo", did());
    json_obj.insert("collection", "app.bsky.feed.post");
    json_obj.insert("record", json_record);
    QJsonDocument json_doc(json_obj);

    AccessAtProtocol::post(QStringLiteral("xrpc/com.atproto.repo.createRecord"),
                           json_doc.toJson(QJsonDocument::Compact));
}

void ComAtprotoRepoCreateRecord::like(const QString &cid, const QString &uri)
{

    QJsonObject json_subject;
    json_subject.insert("cid", cid);
    json_subject.insert("uri", uri);
    QJsonObject json_record;
    json_record.insert("subject", json_subject);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    QJsonObject json_obj;
    json_obj.insert("repo", did());
    json_obj.insert("collection", "app.bsky.feed.like");
    json_obj.insert("record", json_record);
    QJsonDocument json_doc(json_obj);

    AccessAtProtocol::post(QStringLiteral("xrpc/com.atproto.repo.createRecord"),
                           json_doc.toJson(QJsonDocument::Compact));
}

void ComAtprotoRepoCreateRecord::parseJson(const QString reply_json)
{

    bool success = false;

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (!json_doc.isEmpty()) {
        qDebug() << "uri" << json_doc.object().value("uri").toString();
        qDebug() << "cid" << json_doc.object().value("cid").toString();

        if (json_doc.object().contains("uri")) {
            success = true;
        } else {
            qDebug() << "Fail : ComAtprotoRepoCreateRecord";
            qDebug() << reply_json;
        }
    }

    emit finished(success);
}

}
