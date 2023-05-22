#ifndef USERPROFILE_H
#define USERPROFILE_H

#include "atprotocol/accessatprotocol.h"
//#include "atprotocol/lexicons.h"

#include <QObject>

class UserProfile : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString did READ did WRITE setDid NOTIFY didChanged)
    Q_PROPERTY(QString handle READ handle WRITE setHandle NOTIFY handleChanged)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QString avatar READ avatar WRITE setAvatar NOTIFY avatarChanged)
    Q_PROPERTY(QString banner READ banner WRITE setBanner NOTIFY bannerChanged)
    Q_PROPERTY(int followersCount READ followersCount WRITE setFollowersCount NOTIFY
                       followersCountChanged)
    Q_PROPERTY(int followsCount READ followsCount WRITE setFollowsCount NOTIFY followsCountChanged)
    Q_PROPERTY(int postsCount READ postsCount WRITE setPostsCount NOTIFY postsCountChanged)
    Q_PROPERTY(QString indexedAt READ indexedAt WRITE setIndexedAt NOTIFY indexedAtChanged)

public:
    explicit UserProfile(QObject *parent = nullptr);

    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);
    Q_INVOKABLE void getProfile(const QString &did);

    QString did() const;
    void setDid(const QString &newDid);
    QString handle() const;
    void setHandle(const QString &newHandle);
    QString displayName() const;
    void setDisplayName(const QString &newDisplayName);
    QString description() const;
    void setDescription(const QString &newDescription);
    QString avatar() const;
    void setAvatar(const QString &newAvatar);
    QString banner() const;
    void setBanner(const QString &newBanner);
    int followersCount() const;
    void setFollowersCount(int newFollowersCount);
    int followsCount() const;
    void setFollowsCount(int newFollowsCount);
    int postsCount() const;
    void setPostsCount(int newPostsCount);
    QString indexedAt() const;
    void setIndexedAt(const QString &newIndexedAt);

signals:
    void didChanged();
    void handleChanged();
    void displayNameChanged();
    void descriptionChanged();
    void avatarChanged();
    void bannerChanged();
    void followersCountChanged();
    void followsCountChanged();
    void postsCountChanged();
    void indexedAtChanged();

private:
    AtProtocolInterface::AccountData m_account;
    //    AtProtocolType::AppBskyActorDefs::ProfileViewDetailed m_profile;
    QString m_did;
    QString m_handle;
    QString m_displayName;
    QString m_description;
    QString m_avatar;
    QString m_banner;
    int m_followersCount;
    int m_followsCount;
    int m_postsCount;
    QString m_indexedAt;
};

#endif // USERPROFILE_H
