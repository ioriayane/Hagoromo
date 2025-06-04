#ifndef COMATPROTOREPOCREATERECORDEX_H
#define COMATPROTOREPOCREATERECORDEX_H

#include "atprotocol/com/atproto/repo/comatprotorepocreaterecord.h"

namespace AtProtocolInterface {

class ComAtprotoRepoCreateRecordEx : public ComAtprotoRepoCreateRecord
{
public:
    explicit ComAtprotoRepoCreateRecordEx(QObject *parent = nullptr);

    enum ListPurpose : int {
        Curation,
        Moderation,
    };

    void post(const QString &text);
    void repost(const QString &cid, const QString &uri, const QString &via_cid = QString(),
                const QString &via_uri = QString());
    void like(const QString &cid, const QString &uri, const QString &via_cid = QString(),
              const QString &via_uri = QString());
    void follow(const QString &did);
    void block(const QString &did);
    void blockList(const QString &uri);
    void list(const QString &name, const ListPurpose purpose, const QString &description);
    void listItem(const QString &uri, const QString &did);
    void threadGate(const QString &uri, const AtProtocolType::ThreadGateType type,
                    const QList<AtProtocolType::ThreadGateAllow> &allow_rules);
    void postGate(const QString &uri,
                  const AtProtocolType::AppBskyFeedPostgate::MainEmbeddingRulesType type,
                  const QStringList &detached_uris);

    void setReply(const QString &parent_cid, const QString &parent_uri, const QString &root_cid,
                  const QString &root_uri);
    void setQuote(const QString &cid, const QString &uri);
    void setImageBlobs(const QList<AtProtocolType::Blob> &blobs);
    void setFacets(const QList<AtProtocolType::AppBskyRichtextFacet::Main> &newFacets);
    void setPostLanguages(const QStringList &newPostLanguages);
    void setExternalLink(const QString &uri, const QString &title, const QString &description);
    void setFeedGeneratorLink(const QString &uri, const QString &cid);
    void setSelfLabels(const QStringList &labels);

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ComAtprotoRepoStrongRef::Main m_replyParent;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_replyRoot;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_embedQuote;
    QList<AtProtocolType::Blob> m_embedImageBlobs;
    QList<AtProtocolType::AppBskyRichtextFacet::Main> m_facets;
    QStringList m_postLanguages;
    QString m_externalLinkUri;
    QString m_externalLinkTitle;
    QString m_externalLinkDescription;
    QString m_feedGeneratorLinkUri;
    QString m_feedGeneratorLinkCid;
    QStringList m_selfLabels;
};

}

#endif // COMATPROTOREPOCREATERECORDEX_H
