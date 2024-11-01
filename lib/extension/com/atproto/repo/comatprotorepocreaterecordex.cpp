#include "comatprotorepocreaterecordex.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetprofiles.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using AtProtocolInterface::AppBskyActorGetProfiles;
using namespace AtProtocolType;

namespace AtProtocolInterface {

struct MentionPosition
{
    int start = -1;
    int end = -1;
};

ComAtprotoRepoCreateRecordEx::ComAtprotoRepoCreateRecordEx(QObject *parent)
    : ComAtprotoRepoCreateRecord { parent }
{
}

void ComAtprotoRepoCreateRecordEx::post(const QString &text)
{
    QJsonObject json_record;
    json_record.insert("text", text);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    json_record.insert("space.aoisora.post.via", "Hagoromo");
    if (!m_postLanguages.isEmpty()) {
        QJsonArray json_langs;
        for (const auto &lang : qAsConst(m_postLanguages)) {
            json_langs.append(lang);
        }
        json_record.insert("langs", json_langs);
    }

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

    if (!m_externalLinkUri.isEmpty()) {
        // リンクカードと画像添付は排他
        QJsonObject json_external;
        json_external.insert("uri", m_externalLinkUri);
        json_external.insert("title", m_externalLinkTitle.left(300));
        json_external.insert("description", m_externalLinkDescription.left(1000));
        if (!m_embedImageBlobs.isEmpty()) {
            QJsonObject json_external_thumb;
            setJsonBlob(m_embedImageBlobs.at(0), json_external_thumb);
            json_external.insert("thumb", json_external_thumb);
        }
        json_embed_images.insert("$type", "app.bsky.embed.external");
        json_embed_images.insert("external", json_external);

    } else if (!m_feedGeneratorLinkUri.isEmpty()) {
        // カスタムフィードカードとリンクカード
        QJsonObject json_generator;
        json_generator.insert("uri", m_feedGeneratorLinkUri);
        json_generator.insert("cid", m_feedGeneratorLinkCid);
        json_embed_images.insert("$type", "app.bsky.embed.record");
        json_embed_images.insert("record", json_generator);

    } else if (!m_embedImageBlobs.isEmpty()) {
        QJsonArray json_blobs;
        for (const auto &blob : qAsConst(m_embedImageBlobs)) {
            QJsonObject json_blob;
            QJsonObject json_image;
            QJsonObject json_aspect_ratio;
            setJsonAspectRatio(blob.aspect_ratio, json_aspect_ratio);
            if (!json_aspect_ratio.isEmpty()) {
                json_blob.insert("aspectRatio", json_aspect_ratio);
            }
            setJsonBlob(blob, json_image);
            json_blob.insert("image", json_image);
            json_blob.insert("alt", blob.alt);

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
            json_embed.insert("record", json_quote);
        } else {
            json_embed.insert("$type", "app.bsky.embed.recordWithMedia");
            json_embed.insert("media", json_embed_images);
            QJsonObject json_record2;
            json_record2.insert("$type", "app.bsky.embed.record");
            json_record2.insert("record", json_quote);
            json_embed.insert("record", json_record2);
        }

        json_record.insert("embed", json_embed);

    } else if (!json_embed_images.isEmpty()) {
        json_record.insert("embed", json_embed_images);
    }

    if (!m_facets.isEmpty()) {
        LexiconsTypeUnknown::insertFacetsJson(json_record, m_facets);
    }

    if (!m_selfLabels.isEmpty()) {
        QJsonObject json_labels;
        QJsonArray json_label_vals;
        for (const auto &label : qAsConst(m_selfLabels)) {
            QJsonObject json_val;
            json_val.insert("val", label);
            json_label_vals.append(json_val);
        }
        json_labels.insert("$type", "com.atproto.label.defs#selfLabels");
        json_labels.insert("values", json_label_vals);
        json_record.insert("labels", json_labels);
    }

    createRecord(did(), QStringLiteral("app.bsky.feed.post"), QString(), true, json_record,
                 QString());
}

void ComAtprotoRepoCreateRecordEx::repost(const QString &cid, const QString &uri)
{
    QJsonObject json_subject;
    json_subject.insert("cid", cid);
    json_subject.insert("uri", uri);
    QJsonObject json_record;
    json_record.insert("subject", json_subject);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    createRecord(did(), QStringLiteral("app.bsky.feed.repost"), QString(), true, json_record,
                 QString());
}

void ComAtprotoRepoCreateRecordEx::like(const QString &cid, const QString &uri)
{
    QJsonObject json_subject;
    json_subject.insert("cid", cid);
    json_subject.insert("uri", uri);
    QJsonObject json_record;
    json_record.insert("subject", json_subject);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    createRecord(did(), QStringLiteral("app.bsky.feed.like"), QString(), true, json_record,
                 QString());
}

void ComAtprotoRepoCreateRecordEx::follow(const QString &did)
{
    QJsonObject json_record;
    json_record.insert("subject", did);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    createRecord(this->did(), QStringLiteral("app.bsky.graph.follow"), QString(), true, json_record,
                 QString());
}

void ComAtprotoRepoCreateRecordEx::block(const QString &did)
{
    QJsonObject json_record;
    json_record.insert("subject", did);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    json_record.insert("$type", "app.bsky.graph.block");

    createRecord(this->did(), QStringLiteral("app.bsky.graph.block"), QString(), true, json_record,
                 QString());
}

void ComAtprotoRepoCreateRecordEx::blockList(const QString &uri)
{
    QJsonObject json_record;
    json_record.insert("subject", uri);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    json_record.insert("$type", "app.bsky.graph.listblock");

    createRecord(this->did(), QStringLiteral("app.bsky.graph.listblock"), QString(), true,
                 json_record, QString());
}

void ComAtprotoRepoCreateRecordEx::list(const QString &name, const ListPurpose purpose,
                                        const QString &description)
{
    QString p;
    if (purpose == ComAtprotoRepoCreateRecordEx::Curation) {
        p = "app.bsky.graph.defs#curatelist";
    } else {
        p = "app.bsky.graph.defs#modlist";
    }

    QJsonObject json_record;
    json_record.insert("purpose", p);
    json_record.insert("name", name);
    if (!description.isEmpty())
        json_record.insert("description", description);
    if (!m_embedImageBlobs.isEmpty()) {
        const auto &blob = m_embedImageBlobs.first();

        QJsonObject json_image;
        setJsonBlob(blob, json_image);
        json_record.insert("avatar", json_image);
    }
    // descriptionFacets
    // labels
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    json_record.insert("$type", "app.bsky.graph.list");

    createRecord(this->did(), QStringLiteral("app.bsky.graph.list"), QString(), true, json_record,
                 QString());
}

void ComAtprotoRepoCreateRecordEx::listItem(const QString &uri, const QString &did)
{
    QJsonObject json_record;
    json_record.insert("subject", did);
    json_record.insert("list", uri);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    json_record.insert("$type", "app.bsky.graph.listitem");

    createRecord(this->did(), QStringLiteral("app.bsky.graph.listitem"), QString(), true,
                 json_record, QString());
}

void ComAtprotoRepoCreateRecordEx::threadGate(
        const QString &uri, const AtProtocolType::ThreadGateType type,
        const QList<AtProtocolType::ThreadGateAllow> &allow_rules)
{
    if (!uri.startsWith("at://")) {
        emit finished(false);
        return;
    }

    if (type == ThreadGateType::Everybody) {
        emit finished(false);
        return;
    }

    QString rkey = uri.split("/").last();

    createRecord(this->did(), QStringLiteral("app.bsky.feed.threadgate"), rkey, true,
                 makeThreadGateJsonObject(uri, type, allow_rules), QString());
}

void ComAtprotoRepoCreateRecordEx::postGate(
        const QString &uri, const AtProtocolType::AppBskyFeedPostgate::MainEmbeddingRulesType type,
        const QStringList &detached_uris)
{
    if (!uri.startsWith("at://")) {
        emit finished(false);
        return;
    }
    if (type == AppBskyFeedPostgate::MainEmbeddingRulesType::none && detached_uris.isEmpty()) {
        emit finished(false);
        return;
    }

    QString rkey = uri.split("/").last();

    createRecord(this->did(), QStringLiteral("app.bsky.feed.postgate"), rkey, true,
                 makePostGateJsonObject(uri, type, detached_uris), QString());
}

void ComAtprotoRepoCreateRecordEx::setReply(const QString &parent_cid, const QString &parent_uri,
                                            const QString &root_cid, const QString &root_uri)
{
    m_replyParent.cid = parent_cid;
    m_replyParent.uri = parent_uri;
    m_replyRoot.cid = root_cid;
    m_replyRoot.uri = root_uri;
}

void ComAtprotoRepoCreateRecordEx::setQuote(const QString &cid, const QString &uri)
{
    m_embedQuote.cid = cid;
    m_embedQuote.uri = uri;
}

void ComAtprotoRepoCreateRecordEx::setImageBlobs(const QList<AtProtocolType::Blob> &blobs)
{
    m_embedImageBlobs = blobs;
}

bool ComAtprotoRepoCreateRecordEx::parseJson(bool success, const QString reply_json)
{
    success = ComAtprotoRepoCreateRecord::parseJson(success, reply_json);
    if (uri().isEmpty() || cid().isEmpty())
        success = false;

    return success;
}

void ComAtprotoRepoCreateRecordEx::setPostLanguages(const QStringList &newPostLanguages)
{
    m_postLanguages = newPostLanguages;
}

void ComAtprotoRepoCreateRecordEx::setExternalLink(const QString &uri, const QString &title,
                                                   const QString &description)
{
    m_externalLinkUri = uri;
    m_externalLinkTitle = title;
    m_externalLinkDescription = description;
}

void ComAtprotoRepoCreateRecordEx::setFeedGeneratorLink(const QString &uri, const QString &cid)
{
    m_feedGeneratorLinkUri = uri;
    m_feedGeneratorLinkCid = cid;
}

void ComAtprotoRepoCreateRecordEx::setSelfLabels(const QStringList &labels)
{
    m_selfLabels = labels;
}

void ComAtprotoRepoCreateRecordEx::setFacets(
        const QList<AtProtocolType::AppBskyRichtextFacet::Main> &newFacets)
{
    m_facets = newFacets;
}

}
