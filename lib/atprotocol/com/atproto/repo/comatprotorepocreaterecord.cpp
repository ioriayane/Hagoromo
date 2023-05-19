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

    if (!m_replyParent.cid.isEmpty() && !m_replyParent.uri.isEmpty()) {
        QJsonObject json_root;
        if (!m_replyRoot.cid.isEmpty() && !m_replyRoot.uri.isEmpty()) {
            json_root.insert("cid", m_replyRoot.cid);
            json_root.insert("uri", m_replyRoot.uri);
        } else {
            json_root.insert("cid", m_replyParent.cid);
            json_root.insert("uri", m_replyParent.uri);
        }
        QJsonObject json_parent;
        json_parent.insert("cid", m_replyParent.cid);
        json_parent.insert("uri", m_replyParent.uri);
        QJsonObject json_reply;
        json_reply.insert("root", json_root);
        json_reply.insert("parent", json_parent);
        json_record.insert("reply", json_reply);
    }

    QJsonObject json_embed_images;

    if (!m_embedImageBlobs.isEmpty()) {
        QJsonArray json_blobs;
        for (const auto &blob : qAsConst(m_embedImageBlobs)) {
            QJsonObject json_blob;
            QJsonObject json_image;
            json_image.insert("cid", blob.cid);
            json_image.insert("mimeType", blob.mimeType);
            json_image.insert("size", blob.size);
            json_blob.insert("image", json_image);
            json_blob.insert("alt", "");

            json_blobs.append(json_blob);
        }

        json_embed_images.insert("$type", "app.bsky.embed.images");
        json_embed_images.insert("images", json_blobs);
    }

    if (!m_embedQuote.cid.isEmpty() && !m_embedQuote.uri.isEmpty()) {
        QJsonObject json_quote;
        json_quote.insert("cid", m_embedQuote.cid);
        json_quote.insert("uri", m_embedQuote.uri);

        QJsonObject json_embed;
        if (json_embed_images.isEmpty()) {
            json_embed.insert("$type", "app.bsky.embed.record");
        } else {
            json_embed.insert("$type", "app.bsky.embed.recordWithMedia");
            json_embed.insert("media", json_embed_images);
        }
        json_embed.insert("record", json_quote);

        json_record.insert("embed", json_embed);

    } else if (!json_embed_images.isEmpty()) {
        json_record.insert("embed", json_embed_images);
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

void ComAtprotoRepoCreateRecord::setReply(const QString &parent_cid, const QString &parent_uri,
                                          const QString &root_cid, const QString &root_uri)
{
    m_replyParent.cid = parent_cid;
    m_replyParent.uri = parent_uri;
    m_replyRoot.cid = root_cid;
    m_replyRoot.uri = root_uri;
}

void ComAtprotoRepoCreateRecord::setQuote(const QString &cid, const QString &uri)
{
    m_embedQuote.cid = cid;
    m_embedQuote.uri = uri;
}

void ComAtprotoRepoCreateRecord::setImageBlobs(
        const QList<AtProtocolType::LexiconsTypeUnknown::Blob> &blobs)
{
    m_embedImageBlobs = blobs;
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
