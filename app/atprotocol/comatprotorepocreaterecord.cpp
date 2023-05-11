#include "comatprotorepocreaterecord.h"

#include <QDateTime>
#include <QJsonArray>
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

    if (!m_reply.cid.isEmpty() && !m_reply.uri.isEmpty()) {
        QJsonObject json_parent;
        json_parent.insert("cid", m_reply.cid);
        json_parent.insert("uri", m_reply.uri);
        QJsonObject json_reply;
        json_reply.insert("root", json_parent);
        json_reply.insert("parent", json_parent);
        json_record.insert("reply", json_reply);
    }

    QJsonObject json_embed;

    if (!m_embedQuote.cid.isEmpty() && !m_embedQuote.uri.isEmpty()) {
        QJsonObject json_quote;
        json_quote.insert("cid", m_embedQuote.cid);
        json_quote.insert("uri", m_embedQuote.uri);

        json_embed.insert("$type", "app.bsky.embed.record");
        json_embed.insert("record", json_quote);
    }

    if (!json_embed.isEmpty()) {
        json_record.insert("embed", json_embed);
    }

    QJsonObject json_obj;
    json_obj.insert("repo", did());
    json_obj.insert("collection", "app.bsky.feed.post");
    json_obj.insert("record", json_record);
    QJsonDocument json_doc(json_obj);

    AccessAtProtocol::post(QStringLiteral("xrpc/com.atproto.repo.createRecord"),
                           json_doc.toJson(QJsonDocument::Compact));
}

void ComAtprotoRepoCreateRecord::repost(const QString &cid, const QString &uri)
{
    QJsonObject json_subject;
    json_subject.insert("cid", cid);
    json_subject.insert("uri", uri);
    QJsonObject json_record;
    json_record.insert("subject", json_subject);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    QJsonObject json_obj;
    json_obj.insert("repo", did());
    json_obj.insert("collection", "app.bsky.feed.repost");
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

void ComAtprotoRepoCreateRecord::setReply(const QString &cid, const QString &uri)
{
    m_reply.cid = cid;
    m_reply.uri = uri;
}

void ComAtprotoRepoCreateRecord::setQuote(const QString &cid, const QString &uri)
{
    m_embedQuote.cid = cid;
    m_embedQuote.uri = uri;
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
