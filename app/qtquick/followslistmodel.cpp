#include "followslistmodel.h"

#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"

#include <QPointer>

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

    return QVariant();
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
                    beginInsertRows(QModelIndex(), m_didList.count(), m_didList.count());
                    m_profileHash[profile.did] = profile;
                    m_didList.append(profile.did);
                    endInsertRows();
                }
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

    return roles;
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
