#include "comatprotorepocreaterecord.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetprofiles.h"

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

ComAtprotoRepoCreateRecord::ComAtprotoRepoCreateRecord(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ComAtprotoRepoCreateRecord::post(const QString &text)
{
    QJsonObject json_record;
    json_record.insert("text", text);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    json_record.insert("via", "Hagoromo");
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
        QJsonArray json_facets;
        for (const auto &facet : qAsConst(m_facets)) {
            QJsonObject json_facet;
            QJsonObject json_index;
            QJsonArray json_features;
            QJsonObject json_feature;

            json_index.insert("byteStart", facet.index.byteStart);
            json_index.insert("byteEnd", facet.index.byteEnd);
            if (facet.features_type == AppBskyRichtextFacet::MainFeaturesType::features_Link
                && !facet.features_Link.isEmpty()) {
                json_feature.insert("uri", facet.features_Link.first().uri);
                json_feature.insert("$type", "app.bsky.richtext.facet#link");
            } else if (facet.features_type
                               == AppBskyRichtextFacet::MainFeaturesType::features_Mention
                       && !facet.features_Mention.isEmpty()) {
                json_facet.insert("$type", "app.bsky.richtext.facet");
                json_feature.insert("did", facet.features_Mention.first().did);
                json_feature.insert("$type", "app.bsky.richtext.facet#mention");
            } else if (facet.features_type == AppBskyRichtextFacet::MainFeaturesType::features_Tag
                       && !facet.features_Tag.isEmpty()) {
                json_feature.insert("tag", facet.features_Tag.first().tag);
                json_feature.insert("$type", "app.bsky.richtext.facet#tag");
            }
            json_facet.insert("index", json_index);
            json_features.append(json_feature);
            json_facet.insert("features", json_features);
            json_facets.append(json_facet);
        }
        json_record.insert("facets", json_facets);
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

void ComAtprotoRepoCreateRecord::follow(const QString &did)
{
    QJsonObject json_record;
    json_record.insert("subject", did);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    QJsonObject json_obj;
    json_obj.insert("repo", this->did());
    json_obj.insert("collection", "app.bsky.graph.follow");
    json_obj.insert("record", json_record);
    QJsonDocument json_doc(json_obj);

    AccessAtProtocol::post(QStringLiteral("xrpc/com.atproto.repo.createRecord"),
                           json_doc.toJson(QJsonDocument::Compact));
}

void ComAtprotoRepoCreateRecord::block(const QString &did)
{
    QJsonObject json_record;
    json_record.insert("subject", did);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    json_record.insert("$type", "app.bsky.graph.block");
    QJsonObject json_obj;
    json_obj.insert("repo", this->did());
    json_obj.insert("collection", "app.bsky.graph.block");
    json_obj.insert("record", json_record);
    QJsonDocument json_doc(json_obj);

    AccessAtProtocol::post(QStringLiteral("xrpc/com.atproto.repo.createRecord"),
                           json_doc.toJson(QJsonDocument::Compact));
}

void ComAtprotoRepoCreateRecord::list(const QString &name, const ListPurpose purpose,
                                      const QString &description)
{
    QString p;
    if (purpose == ComAtprotoRepoCreateRecord::Curation) {
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
    QJsonObject json_obj;
    json_obj.insert("repo", this->did());
    json_obj.insert("collection", "app.bsky.graph.list");
    json_obj.insert("record", json_record);
    QJsonDocument json_doc(json_obj);

    AccessAtProtocol::post(QStringLiteral("xrpc/com.atproto.repo.createRecord"),
                           json_doc.toJson(QJsonDocument::Compact));
}

void ComAtprotoRepoCreateRecord::listItem(const QString &uri, const QString &did)
{
    QJsonObject json_record;
    json_record.insert("subject", did);
    json_record.insert("list", uri);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    json_record.insert("$type", "app.bsky.graph.listitem");
    QJsonObject json_obj;
    json_obj.insert("repo", this->did());
    json_obj.insert("collection", "app.bsky.graph.listitem");
    json_obj.insert("record", json_record);
    QJsonDocument json_doc(json_obj);

    AccessAtProtocol::post(QStringLiteral("xrpc/com.atproto.repo.createRecord"),
                           json_doc.toJson(QJsonDocument::Compact));
}

void ComAtprotoRepoCreateRecord::threadGate(
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

    QJsonArray json_allow;
    if (type == ThreadGateType::Choice) {
        for (const auto &allow : allow_rules) {
            QJsonObject json_rule;
            if (allow.type == ThreadGateAllowType::Mentioned) {
                json_rule.insert("$type", "app.bsky.feed.threadgate#mentionRule");
            } else if (allow.type == ThreadGateAllowType::Followed) {
                json_rule.insert("$type", "app.bsky.feed.threadgate#followingRule");
            } else if (allow.type == ThreadGateAllowType::List && allow.uri.startsWith("at://")) {
                json_rule.insert("$type", "app.bsky.feed.threadgate#listRule");
                json_rule.insert("list", allow.uri);
            }
            json_allow.append(json_rule);
        }
    }

    QJsonObject json_record;
    json_record.insert("$type", "app.bsky.feed.threadgate");
    json_record.insert("post", uri);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    json_record.insert("allow", json_allow);

    QJsonObject json_obj;
    json_obj.insert("collection", "app.bsky.feed.threadgate");
    json_obj.insert("repo", this->did());
    json_obj.insert("rkey", rkey);
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

void ComAtprotoRepoCreateRecord::setImageBlobs(const QList<AtProtocolType::Blob> &blobs)
{
    m_embedImageBlobs = blobs;
}

bool ComAtprotoRepoCreateRecord::parseJson(bool success, const QString reply_json)
{
    m_replyUri.clear();
    m_replyCid.clear();

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        if (json_doc.object().contains("uri")) {
            m_replyUri = json_doc.object().value("uri").toString();
            m_replyCid = json_doc.object().value("cid").toString();
            qDebug() << "uri" << m_replyUri;
            qDebug() << "cid" << m_replyCid;
        } else {
            qDebug() << "Fail : ComAtprotoRepoCreateRecord";
            qDebug() << reply_json;
            success = false;
        }
    }

    return success;
}

void ComAtprotoRepoCreateRecord::setPostLanguages(const QStringList &newPostLanguages)
{
    m_postLanguages = newPostLanguages;
}

void ComAtprotoRepoCreateRecord::setExternalLink(const QString &uri, const QString &title,
                                                 const QString &description)
{
    m_externalLinkUri = uri;
    m_externalLinkTitle = title;
    m_externalLinkDescription = description;
}

void ComAtprotoRepoCreateRecord::setFeedGeneratorLink(const QString &uri, const QString &cid)
{
    m_feedGeneratorLinkUri = uri;
    m_feedGeneratorLinkCid = cid;
}

void ComAtprotoRepoCreateRecord::setSelfLabels(const QStringList &labels)
{
    m_selfLabels = labels;
}

void ComAtprotoRepoCreateRecord::setFacets(
        const QList<AtProtocolType::AppBskyRichtextFacet::Main> &newFacets)
{
    m_facets = newFacets;
}

QString ComAtprotoRepoCreateRecord::replyUri() const
{
    return m_replyUri;
}

QString ComAtprotoRepoCreateRecord::replyCid() const
{
    return m_replyCid;
}

}
