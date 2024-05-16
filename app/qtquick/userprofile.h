#ifndef USERPROFILE_H
#define USERPROFILE_H

#include "atprotocol/accessatprotocol.h"
// #include "atprotocol/lexicons.h"
#include "systemtool.h"
#include "tools/configurablelabels.h"

#include <QObject>

class UserProfile : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

    Q_PROPERTY(QString did READ did WRITE setDid NOTIFY didChanged)
    Q_PROPERTY(QString handle READ handle WRITE setHandle NOTIFY handleChanged)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QString formattedDescription READ formattedDescription NOTIFY
                       formattedDescriptionChanged)
    Q_PROPERTY(QString avatar READ avatar WRITE setAvatar NOTIFY avatarChanged)
    Q_PROPERTY(QString banner READ banner WRITE setBanner NOTIFY bannerChanged)
    Q_PROPERTY(QString serviceEndpoint READ serviceEndpoint WRITE setServiceEndpoint NOTIFY
                       serviceEndpointChanged)

    Q_PROPERTY(int followersCount READ followersCount WRITE setFollowersCount NOTIFY
                       followersCountChanged)
    Q_PROPERTY(int followsCount READ followsCount WRITE setFollowsCount NOTIFY followsCountChanged)
    Q_PROPERTY(int postsCount READ postsCount WRITE setPostsCount NOTIFY postsCountChanged)
    Q_PROPERTY(QString indexedAt READ indexedAt WRITE setIndexedAt NOTIFY indexedAtChanged)

    Q_PROPERTY(bool following READ following WRITE setFollowing NOTIFY followingChanged)
    Q_PROPERTY(bool followedBy READ followedBy WRITE setFollowedBy NOTIFY followedByChanged)
    Q_PROPERTY(
            QString followingUri READ followingUri WRITE setFollowingUri NOTIFY followingUriChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(bool blockedBy READ blockedBy WRITE setBlockedBy NOTIFY blockedByChanged)
    Q_PROPERTY(bool blocking READ blocking WRITE setBlocking NOTIFY blockingChanged)
    Q_PROPERTY(QString blockingUri READ blockingUri WRITE setBlockingUri NOTIFY blockingUriChanged)

    Q_PROPERTY(bool userFilterMatched READ userFilterMatched WRITE setUserFilterMatched NOTIFY
                       userFilterMatchedChanged)
    Q_PROPERTY(QString userFilterTitle READ userFilterTitle WRITE setUserFilterTitle NOTIFY
                       userFilterTitleChanged)

    Q_PROPERTY(QStringList belongingLists READ belongingLists WRITE setBelongingLists NOTIFY
                       belongingListsChanged)
    Q_PROPERTY(QString pinnedPost READ pinnedPost WRITE setPinnedPost NOTIFY pinnedPostChanged)
public:
    explicit UserProfile(QObject *parent = nullptr);
    ~UserProfile();

    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);
    Q_INVOKABLE void getProfile(const QString &did);

    bool running() const;
    void setRunning(bool newRunning);
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
    bool following() const;
    void setFollowing(bool newFollowing);
    bool followedBy() const;
    void setFollowedBy(bool newFollowedBy);
    QString followingUri() const;
    void setFollowingUri(const QString &newFollowingUri);
    bool muted() const;
    void setMuted(bool newMuted);
    bool blockedBy() const;
    void setBlockedBy(bool newBlockedBy);
    bool blocking() const;
    void setBlocking(bool newBlocking);
    QString blockingUri() const;
    void setBlockingUri(const QString &newBlockingUri);
    bool userFilterMatched() const;
    void setUserFilterMatched(bool newUserFilterMatched);
    QString userFilterTitle() const;
    void setUserFilterTitle(const QString &newUserFilterTitle);
    QStringList belongingLists() const;
    void setBelongingLists(const QStringList &newBelongingLists);

    QString formattedDescription() const;

    QString pinnedPost() const;
    void setPinnedPost(const QString &newPinnedPost);

    QString serviceEndpoint() const;
    void setServiceEndpoint(const QString &newServiceEndpoint);

signals:
    void errorOccured(const QString &code, const QString &message);
    void runningChanged();
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
    void followingChanged();
    void followedByChanged();
    void followingUriChanged();
    void mutedChanged();
    void blockedByChanged();
    void blockingChanged();
    void blockingUriChanged();
    void userFilterMatchedChanged();
    void userFilterTitleChanged();
    void belongingListsChanged();

    void formattedDescriptionChanged();
    void pinnedPostChanged();
    void serviceEndpointChanged();

public slots:
    void updatedBelongingLists(const QString &account_did, const QString &user_did);

private:
    void updateContentFilterLabels(std::function<void()> callback);
    void getServiceEndpoint(const QString &did,
                            std::function<void(const QString &service_endpoint)> callback);
    void getRawProfile();

    SystemTool m_systemTool;
    AtProtocolInterface::AccountData m_account;
    ConfigurableLabels m_contentFilterLabels;
    //    AtProtocolType::AppBskyActorDefs::ProfileViewDetailed m_profile;
    bool m_running;
    QString m_formattedDescription;

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
    bool m_following;
    bool m_followedBy;
    QString m_followingUri;
    bool m_muted;
    bool m_blockedBy;
    bool m_blocking;
    QString m_blockingUri;
    bool m_userFilterMatched;
    QString m_userFilterTitle;
    QStringList m_belongingLists;
    QString m_pinnedPost;
    QString m_serviceEndpoint;
};

#endif // USERPROFILE_H
