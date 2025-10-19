#ifndef USERPROFILE_H
#define USERPROFILE_H

#include "atprotocol/accessatprotocol.h"
// #include "atprotocol/lexicons.h"
#include "systemtool.h"
#include "tools/labelprovider.h"

#include <QObject>

struct HistoryItem
{
    QString date;
    QString handle;
    QString endpoint;
};

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
    Q_PROPERTY(QString website READ website WRITE setWebsite NOTIFY websiteChanged FINAL)
    Q_PROPERTY(QString pronouns READ pronouns WRITE setPronouns NOTIFY pronounsChanged FINAL)

    Q_PROPERTY(int followersCount READ followersCount WRITE setFollowersCount NOTIFY
                       followersCountChanged)
    Q_PROPERTY(int followsCount READ followsCount WRITE setFollowsCount NOTIFY followsCountChanged)
    Q_PROPERTY(int postsCount READ postsCount WRITE setPostsCount NOTIFY postsCountChanged)
    Q_PROPERTY(QString indexedAt READ indexedAt WRITE setIndexedAt NOTIFY indexedAtChanged)

    Q_PROPERTY(bool following READ following WRITE setFollowing NOTIFY followingChanged)
    Q_PROPERTY(bool followedBy READ followedBy WRITE setFollowedBy NOTIFY followedByChanged)
    Q_PROPERTY(
            QString followingUri READ followingUri WRITE setFollowingUri NOTIFY followingUriChanged)
    Q_PROPERTY(QStringList knownFollowerAvators READ knownFollowerAvators WRITE
                       setKnownFollowerAvators NOTIFY knownFollowerAvatorsChanged FINAL)
    Q_PROPERTY(QStringList knownFollowers READ knownFollowers WRITE setKnownFollowers NOTIFY
                       knownFollowersChanged FINAL)
    Q_PROPERTY(int knownFollowersCount READ knownFollowersCount WRITE setKnownFollowersCount NOTIFY
                       knownFollowersCountChanged FINAL)

    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(bool blockedBy READ blockedBy WRITE setBlockedBy NOTIFY blockedByChanged)
    Q_PROPERTY(bool blocking READ blocking WRITE setBlocking NOTIFY blockingChanged)
    Q_PROPERTY(QString blockingUri READ blockingUri WRITE setBlockingUri NOTIFY blockingUriChanged)

    Q_PROPERTY(QStringList labels READ labels WRITE setLabels NOTIFY labelsChanged FINAL)
    Q_PROPERTY(QStringList labelIcons READ labelIcons WRITE setLabelIcons NOTIFY labelIconsChanged
                       FINAL)
    Q_PROPERTY(QStringList belongingLists READ belongingLists WRITE setBelongingLists NOTIFY
                       belongingListsChanged)
    Q_PROPERTY(QString pinnedPost READ pinnedPost WRITE setPinnedPost NOTIFY pinnedPostChanged)
    Q_PROPERTY(QString registrationDate READ registrationDate WRITE setRegistrationDate NOTIFY
                       registrationDateChanged)
    Q_PROPERTY(QStringList handleHistory READ handleHistory WRITE setHandleHistory NOTIFY
                       handleHistoryChanged)
    Q_PROPERTY(bool associatedChatAllow READ associatedChatAllow WRITE setAssociatedChatAllow NOTIFY
                       associatedChatAllowChanged FINAL)

    Q_PROPERTY(QString verificationState READ verificationState WRITE setVerificationState NOTIFY
                       verificationStateChanged FINAL)
    Q_PROPERTY(QStringList verifierList READ verifierList WRITE setVerifierList NOTIFY
                       verifierListChanged FINAL)

    Q_PROPERTY(bool liveIsActive READ liveIsActive WRITE setLiveIsActive NOTIFY liveIsActiveChanged
                       FINAL)
    Q_PROPERTY(QString liveLinkUri READ liveLinkUri WRITE setLiveLinkUri NOTIFY liveLinkUriChanged
                       FINAL)
    Q_PROPERTY(QString liveLinkTitle READ liveLinkTitle WRITE setLiveLinkTitle NOTIFY
                       liveLinkTitleChanged FINAL)
    Q_PROPERTY(QString liveLinkDescription READ liveLinkDescription WRITE setLiveLinkDescription
                       NOTIFY liveLinkDescriptionChanged FINAL)
    Q_PROPERTY(QString liveLinkThumb READ liveLinkThumb WRITE setLiveLinkThumb NOTIFY
                       liveLinkThumbChanged FINAL)
    Q_PROPERTY(QString liveExpiresAt READ liveExpiresAt WRITE setLiveExpiresAt NOTIFY
                       liveExpiresAtChanged FINAL)

    Q_PROPERTY(bool allowSubscriptions READ allowSubscriptions WRITE setAllowSubscriptions NOTIFY
                       allowSubscriptionsChanged FINAL)
    Q_PROPERTY(bool activitySubscriptionPost READ activitySubscriptionPost WRITE
                       setActivitySubscriptionPost NOTIFY activitySubscriptionPostChanged FINAL)
    Q_PROPERTY(bool activitySubscriptionReply READ activitySubscriptionReply WRITE
                       setActivitySubscriptionReply NOTIFY activitySubscriptionReplyChanged FINAL)

public:
    explicit UserProfile(QObject *parent = nullptr);
    ~UserProfile();

    Q_INVOKABLE void setAccount(const QString &uuid);
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
    QStringList knownFollowerAvators() const;
    void setKnownFollowerAvators(const QStringList &newKnownFollowerAvators);
    QStringList knownFollowers() const;
    void setKnownFollowers(const QStringList &newKnownFollowers);
    int knownFollowersCount() const;
    void setKnownFollowersCount(int newKnownFollowersCount);

    bool muted() const;
    void setMuted(bool newMuted);
    bool blockedBy() const;
    void setBlockedBy(bool newBlockedBy);
    bool blocking() const;
    void setBlocking(bool newBlocking);
    QString blockingUri() const;
    void setBlockingUri(const QString &newBlockingUri);
    QStringList labels() const;
    void setLabels(const QStringList &newLabels);
    QStringList labelIcons() const;
    void setLabelIcons(const QStringList &newLabelIcons);
    QStringList belongingLists() const;
    void setBelongingLists(const QStringList &newBelongingLists);

    QString formattedDescription() const;

    QString pinnedPost() const;
    void setPinnedPost(const QString &newPinnedPost);
    QString serviceEndpoint() const;
    void setServiceEndpoint(const QString &newServiceEndpoint);
    QString registrationDate() const;
    void setRegistrationDate(const QString &newRegistrationDate);
    QStringList handleHistory() const;
    void setHandleHistory(const QStringList &newHandleHistory);
    bool associatedChatAllow() const;
    void setAssociatedChatAllow(bool newAssociatedChatAllow);

    QString verificationState() const;
    void setVerificationState(const QString &newVerificationState);
    QStringList verifierList() const;
    void setVerifierList(const QStringList &newVerifierList);

    bool liveIsActive() const;
    void setLiveIsActive(bool newLiveIsActive);
    QString liveLinkUri() const;
    void setLiveLinkUri(const QString &newLiveLinkUri);
    QString liveLinkTitle() const;
    void setLiveLinkTitle(const QString &newLiveLinkTitle);
    QString liveLinkDescription() const;
    void setLiveLinkDescription(const QString &newLiveLinkDescription);
    QString liveLinkThumb() const;
    void setLiveLinkThumb(const QString &newLiveLinkThumb);
    QString liveExpiresAt() const;
    void setLiveExpiresAt(const QString &newLiveExpiresAt);

    bool allowSubscriptions() const;
    void setAllowSubscriptions(bool newAllowSubscriptions);

    bool activitySubscriptionPost() const;
    void setActivitySubscriptionPost(bool newActivitySubscriptionPost);
    bool activitySubscriptionReply() const;
    void setActivitySubscriptionReply(bool newActivitySubscriptionReply);

    QString website() const;
    void setWebsite(const QString &newWebsite);

    QString pronouns() const;
    void setPronouns(const QString &newPronouns);

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
    void knownFollowerAvatorsChanged();
    void knownFollowersChanged();
    void knownFollowersCountChanged();
    void mutedChanged();
    void blockedByChanged();
    void blockingChanged();
    void blockingUriChanged();
    void userFilterMatchedChanged();
    void userFilterTitleChanged();

    void labelsChanged();
    void labelIconsChanged();
    void belongingListsChanged();
    void formattedDescriptionChanged();
    void pinnedPostChanged();
    void serviceEndpointChanged();
    void registrationDateChanged();
    void handleHistoryChanged();
    void associatedChatAllowChanged();

    void verificationStateChanged();
    void verifierListChanged();

    void liveIsActiveChanged();
    void liveLinkUriChanged();
    void liveLinkTitleChanged();
    void liveLinkDescriptionChanged();
    void liveLinkThumbChanged();
    void liveExpiresAtChanged();

    void allowSubscriptionsChanged();
    void activitySubscriptionPostChanged();
    void activitySubscriptionReplyChanged();
    void websiteChanged();

    void pronounsChanged();

public slots:
    void updatedBelongingLists(const QString &account_did, const QString &user_did);
    void finishedConnector(const QString &labeler_did);

private:
    void updateContentFilterLabels(std::function<void()> callback);
    void getServiceEndpoint(const QString &did,
                            std::function<void(const QString &service_endpoint)> callback);
    void getRawInformation(
            const QString &did,
            std::function<void(const QString &, const QString &, const QList<HistoryItem> &)>
                    callback);
    void getRawProfile();
    void getVerifier(const AtProtocolType::AppBskyActorDefs::VerificationState &verification);

    QString labelsTitle(const QString &label, const bool for_image,
                        const QString &labeler_did = QString()) const;
    QStringList labelerDids() const;

    SystemTool m_systemTool;
    AtProtocolInterface::AccountData m_account;
    LabelConnector m_labelConnector;
    //    AtProtocolType::AppBskyActorDefs::ProfileViewDetailed m_profile;
    bool m_running;
    QString m_formattedDescription;
    QList<AtProtocolType::ComAtprotoLabelDefs::Label> m_labelDetails;

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
    QStringList m_knownFollowerAvators;
    QStringList m_knownFollowers;
    int m_knownFollowersCount;
    bool m_muted;
    bool m_blockedBy;
    bool m_blocking;
    QString m_blockingUri;
    bool m_userFilterMatched;
    QString m_userFilterTitle;
    QStringList m_belongingLists;
    QString m_pinnedPost;
    QString m_serviceEndpoint;
    QString m_registrationDate;
    QStringList m_handleHistory;
    bool m_associatedChatAllow;
    QStringList m_labels;
    QStringList m_labelIcons;
    QString m_verificationState;
    QStringList m_verifierList;
    bool m_liveIsActive;
    QString m_liveLinkUri;
    QString m_liveLinkTitle;
    QString m_liveLinkDescription;
    QString m_liveLinkThumb;
    QString m_liveExpiresAt;
    bool m_allowSubscriptions;
    bool m_activitySubscriptionPost;
    bool m_activitySubscriptionReply;
    QString m_website;
    QString m_pronouns;
};

#endif // USERPROFILE_H
