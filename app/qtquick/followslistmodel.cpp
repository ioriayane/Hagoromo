#include "followslistmodel.h"

#include "atprotocol/app/bsky/actor/appbskyactorgetprofiles.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"

#include <QPointer>

using AtProtocolInterface::AppBskyActorGetProfiles;
using AtProtocolInterface::AppBskyGraphGetFollows;

FollowsListModel::FollowsListModel(QObject *parent) : AtpAbstractListModel { parent } { }

int FollowsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_didList.count();
}

QVariant FollowsListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<FollowsListModelRoles>(role));
}

QVariant FollowsListModel::item(int row, FollowsListModelRoles role) const
{
    if (row < 0 || row >= m_didList.count())
        return QVariant();

    const AtProtocolType::AppBskyActorDefs::ProfileView &profile = m_profileHash[m_didList.at(row)];

    if (role == DidRole)
        return profile.did;
    else if (role == HandleRole)
        return profile.handle;
    else if (role == DisplayNameRole)
        return profile.displayName;
    else if (role == DescriptionRole)
        return profile.description;
    else if (role == AvatarRole)
        return profile.avatar;
    else if (role == IndexedAtRole)
        return formatDateTime(profile.indexedAt);
    else if (role == MutedRole)
        return QVariant(); // TODO
    else if (role == BlockedByRole)
        return profile.viewer.blocking;
    else if (role == FollowingRole)
        return profile.viewer.following.contains(account().did);
    else if (role == FollowedByRole)
        return profile.viewer.followedBy.contains(profile.did);
    else if (role == FollowingUriRole)
        return profile.viewer.following;

    return QVariant();
}

void FollowsListModel::remove(const QString &did)
{
    int row = m_didList.indexOf(did);
    if (row == -1)
        return;
    beginRemoveRows(QModelIndex(), row, row);
    m_didList.removeAt(row);
    m_profileHash.remove(did);
    endRemoveRows();
}

int FollowsListModel::indexOf(const QString &cid) const
{
    Q_UNUSED(cid)
    return -1;
}

QString FollowsListModel::getRecordText(const QString &cid)
{
    Q_UNUSED(cid)
    return QString();
}

void FollowsListModel::getProfile(const QString &did)
{
    if (did.isEmpty())
        return;
    if (running())
        return;
    setRunning(true);

    m_cueGetProfile.append(did);
    getProfiles();
}

void FollowsListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    QPointer<FollowsListModel> aliving(this);

    AppBskyGraphGetFollows *follows = new AppBskyGraphGetFollows();
    connect(follows, &AppBskyGraphGetFollows::finished, [=](bool success) {
        if (aliving) {
            if (success) {
                for (const auto &profile : *follows->profileList()) {
                    m_profileHash[profile.did] = profile;
                    if (m_didList.contains(profile.did)) {
                        int row = m_didList.indexOf(profile.did);
                        emit dataChanged(index(row), index(row));
                    } else {
                        beginInsertRows(QModelIndex(), m_didList.count(), m_didList.count());
                        m_didList.append(profile.did);
                        endInsertRows();
                    }
                }
            } else {
                emit errorOccured(follows->errorMessage());
            }
            setRunning(false);
        }
        follows->deleteLater();
    });
    follows->setAccount(account());
    follows->getFollows(targetDid(), 50, QString());
}

QHash<int, QByteArray> FollowsListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[DidRole] = "did";
    roles[HandleRole] = "handle";
    roles[DisplayNameRole] = "displayName";
    roles[DescriptionRole] = "description";
    roles[AvatarRole] = "avatar";
    roles[IndexedAtRole] = "indexedAt";
    roles[MutedRole] = "muted";
    roles[BlockedByRole] = "blockedBy";
    roles[FollowingRole] = "following";
    roles[FollowedByRole] = "followedBy";
    roles[FollowingUriRole] = "followingUri";

    return roles;
}

bool FollowsListModel::checkVisibility(const QString &cid)
{
    Q_UNUSED(cid)
    return true;
}

void FollowsListModel::getProfiles()
{
    if (m_cueGetProfile.isEmpty()) {
        setRunning(false);
        return;
    }

    // getProfilesは最大25個までいっきに取得できる
    QStringList dids;
    for (int i = 0; i < 25; i++) {
        if (m_cueGetProfile.isEmpty())
            break;
        dids.append(m_cueGetProfile.first());
        m_cueGetProfile.removeFirst();
    }

    QPointer<FollowsListModel> aliving(this);

    AppBskyActorGetProfiles *posts = new AppBskyActorGetProfiles();
    connect(posts, &AppBskyActorGetProfiles::finished, [=](bool success) {
        if (aliving) {
            if (success) {
                QStringList new_cid;

                for (auto item = posts->profileViewDetaileds()->crbegin();
                     item != posts->profileViewDetaileds()->crend(); item++) {
                    AtProtocolType::AppBskyActorDefs::ProfileView profile_view;
                    profile_view.avatar = item->avatar;
                    profile_view.did = item->did;
                    profile_view.displayName = item->displayName;
                    profile_view.handle = item->handle;
                    profile_view.description = item->description;
                    profile_view.indexedAt = item->indexedAt;
                    profile_view.labels = item->labels;
                    profile_view.viewer = item->viewer;
                    m_profileHash[item->did] = profile_view;
                    if (m_didList.contains(item->did)) {
                        int r = m_didList.indexOf(item->did);
                        if (r >= 0) {
                            emit dataChanged(index(r), index(r));
                        }
                    } else {
                        beginInsertRows(QModelIndex(), m_didList.count(), m_didList.count());
                        m_didList.append(item->did);
                        endInsertRows();
                    }
                }
            } else {
                emit errorOccured(posts->errorMessage());
            }
            // 残ってたらもう1回
            QTimer::singleShot(100, this, &FollowsListModel::getProfiles);
        }
        posts->deleteLater();
    });
    posts->setAccount(account());
    posts->getProfiles(dids);
}

QString FollowsListModel::targetDid() const
{
    return m_targetDid;
}

void FollowsListModel::setTargetDid(const QString &newTargetDid)
{
    if (m_targetDid == newTargetDid)
        return;
    m_targetDid = newTargetDid;
    emit targetDidChanged();
}
