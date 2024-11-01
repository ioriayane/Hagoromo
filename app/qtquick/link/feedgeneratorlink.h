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
    Q_PROPERTY(QString uri READ uri WRITE setUri NOTIFY uriChanged)
    Q_PROPERTY(QString cid READ cid WRITE setCid NOTIFY cidChanged)
public:
    explicit FeedGeneratorLink(QObject *parent = nullptr);

    AtProtocolInterface::AccountData account() const;
    Q_INVOKABLE void setAccount(const QString &uuid);
    Q_INVOKABLE bool checkUri(const QString &uri, const QString &type) const;
    void convertToAtUri(const QString &base_at_uri, const QString &uri,
                        std::function<void(const QString &)> callback);
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
    QString uri() const;
    void setUri(const QString &newUri);
    QString cid() const;
    void setCid(const QString &newCid);

signals:
    void runningChanged();
    void validChanged();
    void avatarChanged();
    void displayNameChanged();
    void creatorHandleChanged();
    void likeCountChanged();

    void uriChanged();

    void cidChanged();

protected:
private:
    AtProtocolInterface::AccountData m_account;
    QRegularExpression m_rxHandle;

    bool m_running;
    bool m_valid;
    QString m_avatar;
    QString m_displayName;
    QString m_creatorHandle;
    int m_likeCount;
    QString m_uri;
    QString m_cid;
};

#endif // FEEDGENERATORLINK_H
