#ifndef FEEDGENERATORLINK_H
#define FEEDGENERATORLINK_H

#include "atprotocol/accessatprotocol.h"
#include <QObject>

class FeedGeneratorLink : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(bool valid READ valid WRITE setValid NOTIFY validChanged)

    Q_PROPERTY(QString avatar READ avatar WRITE setAvatar NOTIFY avatarChanged)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(QString creatorHandle READ creatorHandle WRITE setCreatorHandle NOTIFY
                       creatorHandleChanged)
    Q_PROPERTY(int likeCount READ likeCount WRITE setLikeCount NOTIFY likeCountChanged)
public:
    explicit FeedGeneratorLink(QObject *parent = nullptr);

    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);
    Q_INVOKABLE bool checkUri(const QString &uri) const;
    Q_INVOKABLE QString convertToAtUri(const QString &uri);
    Q_INVOKABLE void getFeedGenerator(const QString &uri);
    Q_INVOKABLE void clear();

    bool running() const;
    void setRunning(bool newRunning);
    bool valid() const;
    void setValid(bool newValid);
    QString avatar() const;
    void setAvatar(const QString &newAvatar);
    QString displayName() const;
    void setDisplayName(const QString &newDisplayName);
    QString creatorHandle() const;
    void setCreatorHandle(const QString &newCreatorHandle);
    int likeCount() const;
    void setLikeCount(int newLikeCount);

signals:
    void runningChanged();
    void validChanged();
    void avatarChanged();
    void displayNameChanged();
    void creatorHandleChanged();
    void likeCountChanged();

private:
    AtProtocolInterface::AccountData m_account;
    bool m_running;
    bool m_valid;
    QString m_avatar;
    QString m_displayName;
    QString m_creatorHandle;
    int m_likeCount;
};

#endif // FEEDGENERATORLINK_H
