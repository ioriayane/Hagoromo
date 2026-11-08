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
    createRecord(did(), QStringLiteral("app.bsky.feed.post"), QString(), true,
                 makePostJsonObject(text, m_postData), QString());
}

void ComAtprotoRepoCreateRecordEx::repost(const QString &cid, const QString &uri,
                                          const QString &via_cid, const QString &via_uri)
{
    QJsonObject json_subject;
    json_subject.insert("cid", cid);
    json_subject.insert("uri", uri);
    QJsonObject json_record;
    if (!via_cid.isEmpty() && !via_uri.isEmpty()) {
        QJsonObject json_via;
        json_via.insert("cid", via_cid);
        json_via.insert("uri", via_uri);
        json_record.insert("via", json_via);
    }
    json_record.insert("subject", json_subject);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    createRecord(did(), QStringLiteral("app.bsky.feed.repost"), QString(), true, json_record,
                 QString());
}

void ComAtprotoRepoCreateRecordEx::like(const QString &cid, const QString &uri,
                                        const QString &via_cid, const QString &via_uri)
{
    QJsonObject json_subject;
    json_subject.insert("cid", cid);
    json_subject.insert("uri", uri);
    QJsonObject json_record;
    if (!via_cid.isEmpty() && !via_uri.isEmpty()) {
        QJsonObject json_via;
        json_via.insert("cid", via_cid);
        json_via.insert("uri", via_uri);
        json_record.insert("via", json_via);
    }
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
    if (!m_postData.embed_image_blobs.isEmpty()) {
        const auto &blob = m_postData.embed_image_blobs.first();

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

void ComAtprotoRepoCreateRecordEx::tokimekiPoll(const QString &post_uri, const QString &post_cid,
                                                const QStringList &options, const int duration,
                                                const QString &rkey)
{
    if (!post_uri.startsWith("at://")) {
        emit finished(false);
        return;
    }

    createRecord(this->did(), QStringLiteral("tech.tokimeki.poll.poll"), rkey, false,
                 makeTokimekiPollJsonObject(post_uri, post_cid, options, duration), QString());
}

void ComAtprotoRepoCreateRecordEx::profileLink(const QString &url, const QString &title,
                                               const AtProtocolType::Blob &icon)
{
    QJsonObject json_record;
    json_record.insert("$type", "app.bsky.actor.link");
    json_record.insert("url", url);
    if (!title.isEmpty()) {
        json_record.insert("title", title);
    }
    if (!icon.cid.isEmpty()) {
        QJsonObject json_icon;
        setJsonBlob(icon, json_icon);
        json_record.insert("icon", json_icon);
    }
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    createRecord(this->did(), QStringLiteral("app.bsky.actor.link"), QString(), true, json_record,
                 QString());
}

void ComAtprotoRepoCreateRecordEx::setReply(const QString &parent_cid, const QString &parent_uri,
                                            const QString &root_cid, const QString &root_uri)
{
    m_postData.reply_parent.cid = parent_cid;
    m_postData.reply_parent.uri = parent_uri;
    m_postData.reply_root.cid = root_cid;
    m_postData.reply_root.uri = root_uri;
}

void ComAtprotoRepoCreateRecordEx::setQuote(const QString &cid, const QString &uri)
{
    m_postData.embed_quote.cid = cid;
    m_postData.embed_quote.uri = uri;
}

void ComAtprotoRepoCreateRecordEx::setImageBlobs(const QList<AtProtocolType::Blob> &blobs)
{
    m_postData.embed_image_blobs = blobs;
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
    m_postData.post_languages = newPostLanguages;
}

void ComAtprotoRepoCreateRecordEx::setExternalLink(const QString &uri, const QString &title,
                                                   const QString &description)
{
    m_postData.external_link_uri = uri;
    m_postData.external_link_title = title;
    m_postData.external_link_description = description;
}

void ComAtprotoRepoCreateRecordEx::setFeedGeneratorLink(const QString &uri, const QString &cid)
{
    m_postData.feed_generator_link_uri = uri;
    m_postData.feed_generator_link_cid = cid;
}

void ComAtprotoRepoCreateRecordEx::setSelfLabels(const QStringList &labels)
{
    m_postData.self_labels = labels;
}

void ComAtprotoRepoCreateRecordEx::setFacets(
        const QList<AtProtocolType::AppBskyRichtextFacet::Main> &newFacets)
{
    m_postData.facets = newFacets;
}

}
