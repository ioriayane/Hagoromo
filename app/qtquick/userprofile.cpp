#include "userprofile.h"
#include "tools/listitemscache.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetprofile.h"
#include "extension/com/atproto/repo/comatprotorepogetrecordex.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "extension/directory/plc/directoryplc.h"
#include "extension/directory/plc/directoryplclogaudit.h"

#include <QPointer>

using AtProtocolInterface::AppBskyActorGetProfile;
using AtProtocolInterface::ComAtprotoRepoGetRecordEx;
using AtProtocolInterface::DirectoryPlc;
using AtProtocolInterface::DirectoryPlcLogAudit;

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
    QPointer<UserProfile> alive = this;
    connect(ListItemsCache::getInstance(), &ListItemsCache::updated, this,
            &UserProfile::updatedBelongingLists);
}

UserProfile::~UserProfile()
{
    disconnect(ListItemsCache::getInstance(), &ListItemsCache::updated, this,
               &UserProfile::updatedBelongingLists);
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

    updateContentFilterLabels([=]() {
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
                    QString title;
                    QString val;
                    for (const auto &label : detail.labels) {
                        if (label.val == QStringLiteral("!no-unauthenticated"))
                            continue;
                        val = label.val;
                        title = m_contentFilterLabels.title(label.val, false);
                        break;
                    }
                    if (val.isEmpty()) {
                        setUserFilterMatched(false);
                        setUserFilterTitle(QString());
                    } else {
                        setUserFilterMatched(true);
                        if (title.isEmpty()) {
                            setUserFilterTitle(val);
                        } else {
                            setUserFilterTitle(title);
                        }
                    }
                }
                setBelongingLists(
                        ListItemsCache::getInstance()->getListNames(m_account.did, detail.did));

                //追加情報読み込み
                getRawProfile();
            } else {
                emit errorOccured(profile->errorCode(), profile->errorMessage());
                setRunning(false);
            }
            profile->deleteLater();
        });
        profile->setAccount(m_account);
        profile->setLabelers(m_contentFilterLabels.labelerDids());
        profile->getProfile(did);
    });
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
    m_formattedDescription = m_systemTool.markupText(m_description);
    emit descriptionChanged();
    emit formattedDescriptionChanged();
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

QString UserProfile::formattedDescription() const
{
    return m_formattedDescription;
}

void UserProfile::updatedBelongingLists(const QString &account_did, const QString &user_did)
{
    if (m_account.did == account_did && did() == user_did) {
        setBelongingLists(ListItemsCache::getInstance()->getListNames(account_did, user_did));
    }
}

void UserProfile::updateContentFilterLabels(std::function<void()> callback)
{
    ConfigurableLabels *labels = new ConfigurableLabels(this);
    connect(labels, &ConfigurableLabels::finished, this, [=](bool success) {
        if (success) {
            m_contentFilterLabels = *labels;
        }
        callback();
        labels->deleteLater();
    });
    labels->setAccount(m_account);
    labels->load();
}

void UserProfile::getServiceEndpoint(const QString &did,
                                     std::function<void(const QString &)> callback)
{
    if (did.isEmpty()) {
        callback(QString());
        return;
    }

    DirectoryPlc *plc = new DirectoryPlc(this);
    connect(plc, &DirectoryPlc::finished, this, [=](bool success) {
        if (success) {
            callback(plc->serviceEndpoint());
        } else {
            callback(QString());
        }
        plc->deleteLater();
    });
    plc->directory(did);
}

void UserProfile::getRawInformation(
        const QString &did,
        std::function<void(const QString &service_endpoint, const QString &registration_date)>
                callback)
{
    if (did.isEmpty()) {
        callback(QString(), QString());
        return;
    }

    DirectoryPlcLogAudit *plc = new DirectoryPlcLogAudit(this);
    connect(plc, &DirectoryPlcLogAudit::finished, this, [=](bool success) {
        if (success) {
            if (plc->plcAuditLog().isEmpty()) {
                callback(QString(), QString());
            } else {
                callback(plc->plcAuditLog()
                                 .last()
                                 .operation_Plc_operation.services.atproto_pds.endpoint,
                         AtProtocolType::LexiconsTypeUnknown::formatDateTime(
                                 plc->plcAuditLog().first().createdAt, true));
            }
        } else {
            callback(QString(), QString());
        }
        plc->deleteLater();
    });
    plc->audit(did);
}

// getProfileの追加読み込みとして動作させる
void UserProfile::getRawProfile()
{
    if (did().isEmpty()) {
        setRunning(false);
        return;
    }

    getRawInformation(
            did(), [=](const QString &service_endpoint, const QString &registration_date) {
                ComAtprotoRepoGetRecordEx *record = new ComAtprotoRepoGetRecordEx(this);
                connect(record, &ComAtprotoRepoGetRecordEx::finished, this, [=](bool success) {
                    if (success) {
                        AtProtocolType::AppBskyActorProfile::Main profile =
                                AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                                        AtProtocolType::AppBskyActorProfile::Main>(record->value());
                        setPinnedPost(profile.pinnedPost);
                    }
                    setRunning(false);
                    record->deleteLater();
                });
                record->setAccount(m_account);
                if (!service_endpoint.isEmpty()) {
                    record->setService(service_endpoint);
                    setServiceEndpoint(service_endpoint);
                } else {
                    // プロフィールを参照されるユーザーのサービスが参照する側と同じとは限らない（bsky.socialだったとしても）
                    setServiceEndpoint(QString());
                }
                setRegistrationDate(registration_date);
                record->profile(did());
            });
}

QStringList UserProfile::belongingLists() const
{
    return m_belongingLists;
}

void UserProfile::setBelongingLists(const QStringList &newBelongingLists)
{
    if (m_belongingLists == newBelongingLists)
        return;
    m_belongingLists = newBelongingLists;
    emit belongingListsChanged();
}

QString UserProfile::pinnedPost() const
{
    return m_pinnedPost;
}

void UserProfile::setPinnedPost(const QString &newPinnedPost)
{
    if (m_pinnedPost == newPinnedPost)
        return;
    m_pinnedPost = newPinnedPost;
    emit pinnedPostChanged();
}

QString UserProfile::serviceEndpoint() const
{
    return m_serviceEndpoint;
}

void UserProfile::setServiceEndpoint(const QString &newServiceEndpoint)
{
    if (m_serviceEndpoint == newServiceEndpoint)
        return;
    m_serviceEndpoint = newServiceEndpoint;
    emit serviceEndpointChanged();
}

QString UserProfile::registrationDate() const
{
    return m_registrationDate;
}

void UserProfile::setRegistrationDate(const QString &newRegistrationDate)
{
    if (m_registrationDate == newRegistrationDate)
        return;
    m_registrationDate = newRegistrationDate;
    emit registrationDateChanged();
}
