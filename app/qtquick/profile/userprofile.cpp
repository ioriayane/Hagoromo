#include "userprofile.h"
#include "tools/listitemscache.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetprofile.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetprofiles.h"
#include "extension/com/atproto/repo/comatprotorepogetrecordex.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "extension/directory/plc/directoryplc.h"
#include "extension/directory/plc/directoryplclogaudit.h"
#include "tools/labelerprovider.h"
#include "tools/accountmanager.h"

#include <QPointer>

using AtProtocolInterface::AppBskyActorGetProfile;
using AtProtocolInterface::AppBskyActorGetProfiles;
using AtProtocolInterface::ComAtprotoRepoGetRecordEx;
using AtProtocolInterface::DirectoryPlc;
using AtProtocolInterface::DirectoryPlcLogAudit;

UserProfile::UserProfile(QObject *parent)
    : QObject { parent },
      m_labelConnector(this),
      m_running(false),
      m_followersCount(0),
      m_followsCount(0),
      m_postsCount(0),
      m_following(false),
      m_followedBy(false),
      m_knownFollowersCount(0),
      m_muted(false),
      m_blockedBy(false),
      m_blocking(false),
      m_userFilterMatched(false),
      m_verificationState("none")
{
    QPointer<UserProfile> alive = this;
    connect(ListItemsCache::getInstance(), &ListItemsCache::updated, this,
            &UserProfile::updatedBelongingLists);
    connect(&m_labelConnector, &LabelConnector::finished, this, &UserProfile::finishedConnector);
}

UserProfile::~UserProfile()
{
    disconnect(&m_labelConnector, &LabelConnector::finished, this, &UserProfile::finishedConnector);
    disconnect(ListItemsCache::getInstance(), &ListItemsCache::updated, this,
               &UserProfile::updatedBelongingLists);
}

void UserProfile::setAccount(const QString &uuid)
{
    m_account.uuid = uuid;
    m_account = AccountManager::getInstance()->getAccount(m_account.uuid);
}

void UserProfile::getProfile(const QString &did)
{
    if (running())
        return;
    setRunning(true);

    m_labelDetails.clear();
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
                QStringList knownFollowerAvators;
                QStringList knownFollowers;
                for (const auto &f : detail.viewer.knownFollowers.followers) {
                    if (f) {
                        knownFollowerAvators.append(f.data()->avatar);
                        knownFollowers.append(f.data()->displayName.isEmpty()
                                                      ? f.data()->handle
                                                      : f.data()->displayName);
                    }
                }
                setKnownFollowerAvators(knownFollowerAvators);
                setKnownFollowers(knownFollowers);
                setKnownFollowersCount(detail.viewer.knownFollowers.count);
                setMuted(detail.viewer.muted);
                setBlockedBy(detail.viewer.blockedBy);
                setBlocking(detail.viewer.blocking.contains(m_account.did));
                setBlockingUri(detail.viewer.blocking);
                if (detail.labels.isEmpty()) {
                    setLabels(QStringList());
                    setLabelIcons(QStringList());
                } else {
                    QStringList labels;
                    QStringList labelers;
                    m_labelDetails = detail.labels;
                    for (const auto &label : std::as_const(detail.labels)) {
                        if (label.val == QStringLiteral("!no-unauthenticated"))
                            continue;
                        labels.append(label.val);
                        if (!label.src.isEmpty() && !labelers.contains(label.src)) {
                            labelers.append(label.src);
                        }
                    }
                    setLabels(labels);
                    if (!labelers.isEmpty()) {
                        LabelProvider::getInstance()->update(labelers, m_account,
                                                             &m_labelConnector);
                    }
                }
                setBelongingLists(
                        ListItemsCache::getInstance()->getListNames(m_account.did, detail.did));
                setPinnedPost(detail.pinnedPost.uri);

                if (detail.verification.trustedVerifierStatus == "valid") {
                    setVerificationState("verifier");
                } else if (detail.verification.verifiedStatus == "valid") {
                    setVerificationState("verified");
                } else {
                    setVerificationState("none");
                }

                if (detail.associated.chat.allowIncoming == "none") {
                    setAssociatedChatAllow(false);
                } else if (detail.associated.chat.allowIncoming == "following") {
                    setAssociatedChatAllow(detail.viewer.followedBy.contains(detail.did));
                } else if (detail.associated.chat.allowIncoming == "all") {
                    setAssociatedChatAllow(true);
                } else {
                    // 未設定はフォロー中と同等（2024/5/26）
                    setAssociatedChatAllow(detail.viewer.followedBy.contains(detail.did));
                }

                // 追加情報読み込み
                getRawProfile();
                getVerifier(detail.verification);
            } else {
                emit errorOccured(profile->errorCode(), profile->errorMessage());
                setRunning(false);
            }
            profile->deleteLater();
        });
        profile->setAccount(AccountManager::getInstance()->getAccount(m_account.uuid));
        profile->setLabelers(labelerDids());
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

QStringList UserProfile::knownFollowerAvators() const
{
    return m_knownFollowerAvators;
}

void UserProfile::setKnownFollowerAvators(const QStringList &newKnownFollowerAvators)
{
    if (m_knownFollowerAvators == newKnownFollowerAvators)
        return;
    m_knownFollowerAvators = newKnownFollowerAvators;
    emit knownFollowerAvatorsChanged();
}

QStringList UserProfile::knownFollowers() const
{
    return m_knownFollowers;
}

void UserProfile::setKnownFollowers(const QStringList &newKnownFollowers)
{
    if (m_knownFollowers == newKnownFollowers)
        return;
    m_knownFollowers = newKnownFollowers;
    emit knownFollowersChanged();
}

int UserProfile::knownFollowersCount() const
{
    return m_knownFollowersCount;
}

void UserProfile::setKnownFollowersCount(int newKnownFollowersCount)
{
    if (m_knownFollowersCount == newKnownFollowersCount)
        return;
    m_knownFollowersCount = newKnownFollowersCount;
    emit knownFollowersCountChanged();
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

QString UserProfile::formattedDescription() const
{
    return m_formattedDescription;
}

void UserProfile::updatedBelongingLists(const QString &account_did, const QString &user_did)
{
    if (AccountManager::getInstance()->getAccount(m_account.uuid).did == account_did
        && did() == user_did) {
        setBelongingLists(ListItemsCache::getInstance()->getListNames(account_did, user_did));
    }
}

void UserProfile::finishedConnector(const QString &labeler_did)
{
    Q_UNUSED(labeler_did)
    LabelProvider *provider = LabelProvider::getInstance();
    LabelData label_data;

    QStringList labels;
    QStringList icons;
    for (const auto &label : m_labelDetails) {
        if (label.val == QStringLiteral("!no-unauthenticated"))
            continue;
        label_data = provider->getLabel(label.src, label.val);
        labels.append(label_data.name);
        icons.append(label_data.avatar);
    }
    setLabels(labels);
    setLabelIcons(icons);
}

void UserProfile::updateContentFilterLabels(std::function<void()> callback)
{
    LabelerProvider *provider = LabelerProvider::getInstance();
    LabelerConnector *connector = new LabelerConnector(this);

    connect(connector, &LabelerConnector::finished, this, [=](bool success) {
        if (success) { }
        callback();
        connector->deleteLater();
    });
    provider->setAccount(AccountManager::getInstance()->getAccount(m_account.uuid));
    provider->update(AccountManager::getInstance()->getAccount(m_account.uuid), connector,
                     LabelerProvider::RefleshAuto);
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
        std::function<void(const QString &service_endpoint, const QString &registration_date,
                           const QList<HistoryItem> &handle_history)>
                callback)
{
    if (did.isEmpty()) {
        callback(QString(), QString(), QList<HistoryItem>());
        return;
    }

    DirectoryPlcLogAudit *plc = new DirectoryPlcLogAudit(this);
    connect(plc, &DirectoryPlcLogAudit::finished, this, [=](bool success) {
        if (success) {
            if (plc->plcAuditLog().isEmpty()) {
                callback(QString(), QString(), QList<HistoryItem>());
            } else {
                QList<HistoryItem> history;
                for (const auto &log : plc->plcAuditLog()) {
                    if (log.operation_type
                        == AtProtocolType::DirectoryPlcDefs::PlcAuditLogDetailOperationType::
                                operation_Plc_operation) {
                        for (auto uri : log.operation_Plc_operation.alsoKnownAs) {
                            HistoryItem item;
                            item.date = AtProtocolType::LexiconsTypeUnknown::formatDateTime(
                                    log.createdAt, true);
                            item.handle = uri.remove("at://");
                            item.endpoint =
                                    log.operation_Plc_operation.services.atproto_pds.endpoint;
                            if (history.isEmpty()) {
                                history.append(item);
                            } else {
                                if (history.last().handle != item.handle
                                    || history.last().endpoint != item.endpoint) {
                                    history.append(item);
                                }
                            }
                        }
                    }
                }
                callback(plc->plcAuditLog()
                                 .last()
                                 .operation_Plc_operation.services.atproto_pds.endpoint,
                         AtProtocolType::LexiconsTypeUnknown::formatDateTime(
                                 plc->plcAuditLog().first().createdAt, true),
                         history);
            }
        } else {
            callback(QString(), QString(), QList<HistoryItem>());
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

    getRawInformation(did(),
                      [=](const QString &service_endpoint, const QString &registration_date,
                          const QList<HistoryItem> &handle_history) {
                          if (!service_endpoint.isEmpty()) {
                              setServiceEndpoint(service_endpoint);
                          } else {
                              // プロフィールを参照されるユーザーのサービスが参照する側と同じとは限らない（bsky.socialだったとしても）
                              setServiceEndpoint(QString());
                          }
                          setRegistrationDate(registration_date);
                          QStringList history;
                          for (const auto &item : handle_history) {
                              history.append(item.date);
                              history.append(item.handle);
                              history.append(item.endpoint);
                          }
                          setHandleHistory(history);
                          setRunning(false);
                      });
}

void UserProfile::getVerifier(
        const AtProtocolType::AppBskyActorDefs::VerificationState &verification)
{
    QStringList target_dids;
    QHash<QString, QString> target_created_at;
    for (const auto &verifier : verification.verifications) {
        if (verifier.isValid) {
            target_dids.append(verifier.issuer);
            target_created_at[verifier.issuer] = verifier.createdAt;
        }
    }
    if (target_dids.isEmpty())
        return;

    AppBskyActorGetProfiles *profiles = new AppBskyActorGetProfiles(this);
    connect(profiles, &AppBskyActorGetProfiles::finished, this, [=](bool success) {
        if (success) {
            QStringList verifier_list;
            for (const auto &profile : profiles->profilesList()) {
                verifier_list.append(profile.displayName);
                verifier_list.append(profile.handle);
                verifier_list.append(AtProtocolType::LexiconsTypeUnknown::formatDateTime(
                        target_created_at[profile.did], true));
            }
            setVerifierList(verifier_list);
        } else {
            emit errorOccured(profiles->errorCode(), profiles->errorMessage());
        }
        profiles->deleteLater();
    });
    profiles->setAccount(AccountManager::getInstance()->getAccount(m_account.uuid));
    profiles->getProfiles(target_dids);
}

QString UserProfile::labelsTitle(const QString &label, const bool for_image,
                                 const QString &labeler_did) const
{
    return LabelerProvider::getInstance()->title(
            AccountManager::getInstance()->getAccount(m_account.uuid), label, for_image,
            labeler_did);
}

QStringList UserProfile::labelerDids() const
{
    return LabelerProvider::getInstance()->labelerDids(
            AccountManager::getInstance()->getAccount(m_account.uuid));
}

QStringList UserProfile::labels() const
{
    return m_labels;
}

void UserProfile::setLabels(const QStringList &newLabels)
{
    if (m_labels == newLabels)
        return;
    m_labels = newLabels;
    emit labelsChanged();
}

QStringList UserProfile::labelIcons() const
{
    return m_labelIcons;
}

void UserProfile::setLabelIcons(const QStringList &newLabelIcons)
{
    if (m_labelIcons == newLabelIcons)
        return;
    m_labelIcons = newLabelIcons;
    emit labelIconsChanged();
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

QStringList UserProfile::handleHistory() const
{
    return m_handleHistory;
}

void UserProfile::setHandleHistory(const QStringList &newHandleHistory)
{
    if (m_handleHistory == newHandleHistory)
        return;
    m_handleHistory = newHandleHistory;
    emit handleHistoryChanged();
}

bool UserProfile::associatedChatAllow() const
{
    return m_associatedChatAllow;
}

void UserProfile::setAssociatedChatAllow(bool newAssociatedChatAllow)
{
    if (m_associatedChatAllow == newAssociatedChatAllow)
        return;
    m_associatedChatAllow = newAssociatedChatAllow;
    emit associatedChatAllowChanged();
}

QString UserProfile::verificationState() const
{
    return m_verificationState;
}

void UserProfile::setVerificationState(const QString &newVerificationState)
{
    if (m_verificationState == newVerificationState)
        return;
    m_verificationState = newVerificationState;
    emit verificationStateChanged();
}

QStringList UserProfile::verifierList() const
{
    return m_verifierList;
}

void UserProfile::setVerifierList(const QStringList &newVerifierList)
{
    if (m_verifierList == newVerifierList)
        return;
    m_verifierList = newVerifierList;
    emit verifierListChanged();
}
