#ifndef RECORDOPERATOR_H
#define RECORDOPERATOR_H

#include "atprotocol/lexicons.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "atprotocol/accessatprotocol.h"
#include <QObject>

struct EmbedImage
{
    QString path;
    QString alt;
};

class RecordOperator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

public:
    explicit RecordOperator(QObject *parent = nullptr);

    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);
    Q_INVOKABLE void setText(const QString &text);
    Q_INVOKABLE void setReply(const QString &parent_cid, const QString &parent_uri,
                              const QString &root_cid, const QString &root_uri);
    Q_INVOKABLE void setQuote(const QString &cid, const QString &uri);
    Q_INVOKABLE void setImages(const QStringList &images, const QStringList &alts);
    Q_INVOKABLE void setPostLanguages(const QStringList &langs);
    Q_INVOKABLE void setExternalLink(const QString &uri, const QString &title,
                                     const QString &description, const QString &image_path);
    Q_INVOKABLE void setFeedGeneratorLink(const QString &uri, const QString &cid);
    Q_INVOKABLE void setSelfLabels(const QStringList &labels);

    Q_INVOKABLE void clear();

    Q_INVOKABLE void post();
    Q_INVOKABLE void postWithImages();
    Q_INVOKABLE void repost(const QString &cid, const QString &uri);
    Q_INVOKABLE void like(const QString &cid, const QString &uri);
    Q_INVOKABLE void follow(const QString &did);
    Q_INVOKABLE void mute(const QString &did);
    Q_INVOKABLE void block(const QString &did);

    Q_INVOKABLE void deletePost(const QString &uri);
    Q_INVOKABLE void deleteLike(const QString &uri);
    Q_INVOKABLE void deleteRepost(const QString &uri);
    Q_INVOKABLE void deleteFollow(const QString &uri);
    Q_INVOKABLE void deleteMute(const QString &did);
    Q_INVOKABLE void deleteBlock(const QString &uri);

    bool running() const;
    void setRunning(bool newRunning);

signals:
    void errorOccured(const QString &message);
    void finished(bool success, const QString &uri, const QString &cid);
    void runningChanged();

private:
    template<typename F>
    void makeFacets(const QString &text, F callback);

    QRegularExpression m_rxFacet;
    AtProtocolInterface::AccountData m_account;

    QString m_text;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_replyParent;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_replyRoot;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_embedQuote;
    QList<EmbedImage> m_embedImages;
    QList<AtProtocolType::LexiconsTypeUnknown::Blob> m_embedImageBlogs;
    QList<AtProtocolType::AppBskyRichtextFacet::Main> m_facets;
    QStringList m_postLanguages;
    QString m_externalLinkUri;
    QString m_externalLinkTitle;
    QString m_externalLinkDescription;
    QString m_feedGeneratorLinkUri;
    QString m_feedGeneratorLinkCid;
    QStringList m_selfLabels;

    bool m_running;
};

#endif // RECORDOPERATOR_H
