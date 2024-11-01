#ifndef USERPOST_H
#define USERPOST_H

#include "atprotocol/accessatprotocol.h"
#include "tools/configurablelabels.h"

#include <QObject>

class UserPost : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

    Q_PROPERTY(QString cid READ cid WRITE setCid NOTIFY cidChanged FINAL)
    Q_PROPERTY(QString uri READ uri WRITE setUri NOTIFY uriChanged FINAL)

    Q_PROPERTY(QString authorDid READ authorDid WRITE setAuthorDid NOTIFY authorDidChanged FINAL)
    Q_PROPERTY(QString authorDisplayName READ authorDisplayName WRITE setAuthorDisplayName NOTIFY
                       authorDisplayNameChanged FINAL)
    Q_PROPERTY(QString authorHandle READ authorHandle WRITE setAuthorHandle NOTIFY
                       authorHandleChanged FINAL)
    Q_PROPERTY(QString authorAvatar READ authorAvatar WRITE setAuthorAvatar NOTIFY
                       authorAvatarChanged FINAL)
    Q_PROPERTY(
            bool authorMuted READ authorMuted WRITE setAuthorMuted NOTIFY authorMutedChanged FINAL)
    Q_PROPERTY(
            QString recordText READ recordText WRITE setRecordText NOTIFY recordTextChanged FINAL)
    Q_PROPERTY(QString recordTextPlain READ recordTextPlain WRITE setRecordTextPlain NOTIFY
                       recordTextPlainChanged FINAL)
    Q_PROPERTY(QString indexedAt READ indexedAt WRITE setIndexedAt NOTIFY indexedAtChanged FINAL)
    Q_PROPERTY(QStringList embedImages READ embedImages WRITE setEmbedImages NOTIFY
                       embedImagesChanged FINAL)
    Q_PROPERTY(QStringList embedImagesFull READ embedImagesFull WRITE setEmbedImagesFull NOTIFY
                       embedImagesFullChanged FINAL)
    Q_PROPERTY(QStringList embedImagesAlt READ embedImagesAlt WRITE setEmbedImagesAlt NOTIFY
                       embedImagesAltChanged FINAL)

public:
    explicit UserPost(QObject *parent = nullptr);

    Q_INVOKABLE void setAccount(const QString &uuid);
    Q_INVOKABLE void getPost(const QString &uri);
    Q_INVOKABLE void clear();

    bool running() const;
    void setRunning(bool newRunning);
    QString cid() const;
    void setCid(const QString &newCid);
    QString uri() const;
    void setUri(const QString &newUri);
    QString authorDid() const;
    void setAuthorDid(const QString &newAuthorDid);
    QString authorDisplayName() const;
    void setAuthorDisplayName(const QString &newAuthorDisplayName);
    QString authorHandle() const;
    void setAuthorHandle(const QString &newAuthorHandle);
    QString authorAvatar() const;
    void setAuthorAvatar(const QString &newAuthorAvatar);
    bool authorMuted() const;
    void setAuthorMuted(const bool newAuthorMuted);
    QString recordText() const;
    void setRecordText(const QString &newRecordText);
    QString recordTextPlain() const;
    void setRecordTextPlain(const QString &newRecordTextPlain);
    QString indexedAt() const;
    void setIndexedAt(const QString &newIndexedAt);
    QStringList embedImages() const;
    void setEmbedImages(const QStringList &newEmbedImages);
    QStringList embedImagesFull() const;
    void setEmbedImagesFull(const QStringList &newEmbedImagesFull);
    QStringList embedImagesAlt() const;
    void setEmbedImagesAlt(const QStringList &newEmbedImagesAlt);

signals:
    void errorOccured(const QString &code, const QString &message);

    void runningChanged();
    void cidChanged();
    void uriChanged();
    void authorDidChanged();
    void authorDisplayNameChanged();
    void authorHandleChanged();
    void authorAvatarChanged();
    void authorMutedChanged();
    void recordTextChanged();
    void recordTextPlainChanged();
    void indexedAtChanged();
    void embedImagesChanged();
    void embedImagesFullChanged();
    void embedImagesAltChanged();

private:
    void convertToAtUri(const QString &base_at_uri, const QString &uri,
                        std::function<void(const QString &)> callback);
    QStringList labelerDids() const;

    AtProtocolInterface::AccountData m_account;
    bool m_running;
    QString m_cid;
    QString m_uri;
    QString m_authorDid;
    QString m_authorDisplayName;
    QString m_authorHandle;
    QString m_authorAvatar;
    bool m_authorMuted;
    QString m_recordText;
    QString m_recordTextPlain;
    QString m_indexedAt;
    QStringList m_embedImages;
    QStringList m_embedImagesFull;
    QStringList m_embedImagesAlt;
};

#endif // USERPOST_H
