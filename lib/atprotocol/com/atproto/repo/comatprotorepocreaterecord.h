#ifndef COMATPROTOREPOCREATERECORD_H
#define COMATPROTOREPOCREATERECORD_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"
#include "atprotocol/lexicons_func_unknown.h"

namespace AtProtocolInterface {

class ComAtprotoRepoCreateRecord : public AccessAtProtocol
{
public:
    explicit ComAtprotoRepoCreateRecord(QObject *parent = nullptr);

    enum ListPurpose : int {
        Curation,
        Moderation,
    };

    void post(const QString &text);
    void repost(const QString &cid, const QString &uri);
    void like(const QString &cid, const QString &uri);
    void follow(const QString &did);
    void block(const QString &did);
    bool list(const QString &name, const ListPurpose purpose, const QString &description);
    bool listItem(const QString &uri, const QString &did);

    void setReply(const QString &parent_cid, const QString &parent_uri, const QString &root_cid,
                  const QString &root_uri);
    void setQuote(const QString &cid, const QString &uri);
    void setImageBlobs(const QList<AtProtocolType::LexiconsTypeUnknown::Blob> &blobs);
    void setFacets(const QList<AtProtocolType::AppBskyRichtextFacet::Main> &newFacets);
    void setPostLanguages(const QStringList &newPostLanguages);
    void setExternalLink(const QString &uri, const QString &title, const QString &description);
    void setFeedGeneratorLink(const QString &uri, const QString &cid);
    void setSelfLabels(const QStringList &labels);

    QString replyCid() const;
    QString replyUri() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ComAtprotoRepoStrongRef::Main m_replyParent;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_replyRoot;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_embedQuote;
    QList<AtProtocolType::LexiconsTypeUnknown::Blob> m_embedImageBlobs;
    QList<AtProtocolType::AppBskyRichtextFacet::Main> m_facets;
    QStringList m_postLanguages;
    QString m_externalLinkUri;
    QString m_externalLinkTitle;
    QString m_externalLinkDescription;
    QString m_feedGeneratorLinkUri;
    QString m_feedGeneratorLinkCid;
    QStringList m_selfLabels;

    QString m_replyCid;
    QString m_replyUri;
};

}

#endif // COMATPROTOREPOCREATERECORD_H
