#include "userprofile.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetprofile.h"

#include <QPointer>

using AtProtocolInterface::AppBskyActorGetProfile;

UserProfile::UserProfile(QObject *parent)
    : QObject { parent },
      m_running(false),
      m_followersCount(0),
      m_followsCount(0),
      m_postsCount(0),
      m_following(false),
      m_followedBy(false)
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

    QPointer<UserProfile> aliving(this);

    AppBskyActorGetProfile *profile = new AppBskyActorGetProfile();
    connect(profile, &AppBskyActorGetProfile::finished, [=](bool success) {
        if (aliving) {
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
            } else {
                emit errorOccured(profile->errorMessage());
            }
            setRunning(false);
        }
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
    return m_description;
}

void UserProfile::setDescription(const QString &newDescription)
{
    if (m_description == newDescription)
        return;
    m_description = newDescription;
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
