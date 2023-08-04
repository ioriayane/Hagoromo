#include "userprofile.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetprofile.h"

using AtProtocolInterface::AppBskyActorGetProfile;

UserProfile::UserProfile(QObject *parent)
    : QObject { parent },
      m_running(false),
      m_followersCount(0),
      m_followsCount(0),
      m_postsCount(0),
      m_following(false),
      m_followedBy(false),
      m_muted(false),
      m_blockedBy(false),
      m_blocking(false),
      m_userFilterMatched(false)
{
}

void UserProfile::setAccount(const QString &service, const QString &did, const QString &handle,
                             const QString &email, const QString &accessJwt,
                             const QString &refreshJwt)
{
    m_account.service = service;
    m_account.did = did;
    m_account.handle = handle;
    m_account.email = email;
    m_account.accessJwt = accessJwt;
    m_account.refreshJwt = refreshJwt;
}

void UserProfile::getProfile(const QString &did)
{
    if (running())
        return;
    setRunning(true);

    AppBskyActorGetProfile *profile = new AppBskyActorGetProfile(this);
    connect(profile, &AppBskyActorGetProfile::finished, [=](bool success) {
        if (success) {
            AtProtocolType::AppBskyActorDefs::ProfileViewDetailed detail =
                    profile->profileViewDetailed();
            qDebug() << "Get user profile detailed" << detail.displayName << detail.description;
            setDid(detail.did);
            setHandle(detail.handle);
            setDisplayName(detail.displayName);
            setDescription(detail.description);
            setAvatar(detail.avatar);
            setBanner(detail.banner);
            setFollowersCount(detail.followersCount);
            setFollowsCount(detail.followsCount);
            setPostsCount(detail.postsCount);
            setIndexedAt(QDateTime::fromString(detail.indexedAt, Qt::ISODateWithMs)
                                 .toLocalTime()
                                 .toString("yyyy/MM/dd"));

            setFollowing(detail.viewer.following.contains(m_account.did));
            setFollowedBy(detail.viewer.followedBy.contains(did));
            setFollowingUri(detail.viewer.following);
            setMuted(detail.viewer.muted);
            setBlockedBy(detail.viewer.blockedBy);
            setBlocking(detail.viewer.blocking.contains(m_account.did));
            setBlockingUri(detail.viewer.blocking);
            if (detail.labels.isEmpty()) {
                setUserFilterMatched(false);
                setUserFilterTitle(QString());
            } else {
                setUserFilterMatched(true);
                setUserFilterTitle(m_contentFilterLabels.title(
                        m_contentFilterLabels.indexOf(detail.labels.at(0).val)));
            }
        } else {
            emit errorOccured(profile->errorMessage());
        }
        setRunning(false);
        profile->deleteLater();
    });
    profile->setAccount(m_account);
    profile->getProfile(did);
}

QString UserProfile::did() const
{
    return m_did;
}

void UserProfile::setDid(const QString &newDid)
{
    if (m_did == newDid)
        return;
    m_did = newDid;
    emit didChanged();
}

QString UserProfile::handle() const
{
    return m_handle;
}

void UserProfile::setHandle(const QString &newHandle)
{
    if (m_handle == newHandle)
        return;
    m_handle = newHandle;
    emit handleChanged();
}

QString UserProfile::displayName() const
{
    return m_displayName;
}

void UserProfile::setDisplayName(const QString &newDisplayName)
{
    if (m_displayName == newDisplayName)
        return;
    m_displayName = newDisplayName;
    emit displayNameChanged();
}

QString UserProfile::description() const
{
    return m_formattedDescription; // m_description;
}

void UserProfile::setDescription(const QString &newDescription)
{
    if (m_description == newDescription)
        return;
    m_description = newDescription;
    m_formattedDescription = m_systemTool.markupText(m_description);
    emit descriptionChanged();
}

QString UserProfile::avatar() const
{
    return m_avatar;
}

void UserProfile::setAvatar(const QString &newAvatar)
{
    if (m_avatar == newAvatar)
        return;
    m_avatar = newAvatar;
    emit avatarChanged();
}

QString UserProfile::banner() const
{
    return m_banner;
}

void UserProfile::setBanner(const QString &newBanner)
{
    if (m_banner == newBanner)
        return;
    m_banner = newBanner;
    emit bannerChanged();
}

int UserProfile::followersCount() const
{
    return m_followersCount;
}

void UserProfile::setFollowersCount(int newFollowersCount)
{
    if (m_followersCount == newFollowersCount)
        return;
    m_followersCount = newFollowersCount;
    emit followersCountChanged();
}

int UserProfile::followsCount() const
{
    return m_followsCount;
}

void UserProfile::setFollowsCount(int newFollowsCount)
{
    if (m_followsCount == newFollowsCount)
        return;
    m_followsCount = newFollowsCount;
    emit followsCountChanged();
}

int UserProfile::postsCount() const
{
    return m_postsCount;
}

void UserProfile::setPostsCount(int newPostsCount)
{
    if (m_postsCount == newPostsCount)
        return;
    m_postsCount = newPostsCount;
    emit postsCountChanged();
}

QString UserProfile::indexedAt() const
{
    return m_indexedAt;
}

void UserProfile::setIndexedAt(const QString &newIndexedAt)
{
    if (m_indexedAt == newIndexedAt)
        return;
    m_indexedAt = newIndexedAt;
    emit indexedAtChanged();
}

bool UserProfile::following() const
{
    return m_following;
}

void UserProfile::setFollowing(bool newFollowing)
{
    if (m_following == newFollowing)
        return;
    m_following = newFollowing;
    emit followingChanged();
}

bool UserProfile::followedBy() const
{
    return m_followedBy;
}

void UserProfile::setFollowedBy(bool newFollowedBy)
{
    if (m_followedBy == newFollowedBy)
        return;
    m_followedBy = newFollowedBy;
    emit followedByChanged();
}

QString UserProfile::followingUri() const
{
    return m_followingUri;
}

void UserProfile::setFollowingUri(const QString &newFollowingUri)
{
    if (m_followingUri == newFollowingUri)
        return;
    m_followingUri = newFollowingUri;
    emit followingUriChanged();
}

bool UserProfile::running() const
{
    return m_running;
}

void UserProfile::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

bool UserProfile::muted() const
{
    return m_muted;
}

void UserProfile::setMuted(bool newMuted)
{
    if (m_muted == newMuted)
        return;
    m_muted = newMuted;
    emit mutedChanged();
}

bool UserProfile::blockedBy() const
{
    return m_blockedBy;
}

void UserProfile::setBlockedBy(bool newBlockedBy)
{
    if (m_blockedBy == newBlockedBy)
        return;
    m_blockedBy = newBlockedBy;
    emit blockedByChanged();
}

bool UserProfile::blocking() const
{
    return m_blocking;
}

void UserProfile::setBlocking(bool newBlocking)
{
    if (m_blocking == newBlocking)
        return;
    m_blocking = newBlocking;
    emit blockingChanged();
}

QString UserProfile::blockingUri() const
{
    return m_blockingUri;
}

void UserProfile::setBlockingUri(const QString &newBlockingUri)
{
    if (m_blockingUri == newBlockingUri)
        return;
    m_blockingUri = newBlockingUri;
    emit blockingUriChanged();
}

bool UserProfile::userFilterMatched() const
{
    return m_userFilterMatched;
}

void UserProfile::setUserFilterMatched(bool newUserFilterMatched)
{
    if (m_userFilterMatched == newUserFilterMatched)
        return;
    m_userFilterMatched = newUserFilterMatched;
    emit userFilterMatchedChanged();
}

QString UserProfile::userFilterTitle() const
{
    return m_userFilterTitle;
}

void UserProfile::setUserFilterTitle(const QString &newUserFilterTitle)
{
    if (m_userFilterTitle == newUserFilterTitle)
        return;
    m_userFilterTitle = newUserFilterTitle;
    emit userFilterTitleChanged();
}
