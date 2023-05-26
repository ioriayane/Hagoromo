#include "followerslistmodel.h"

#include "atprotocol/app/bsky/graph/appbskygraphgetfollowers.h"

#include <QPointer>

using AtProtocolInterface::AppBskyGraphGetFollowers;

FollowersListModel::FollowersListModel(QObject *parent) : FollowsListModel { parent } { }

void FollowersListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    QPointer<FollowsListModel> aliving(this);

    AppBskyGraphGetFollowers *followers = new AppBskyGraphGetFollowers();
    connect(followers, &AppBskyGraphGetFollowers::finished, [=](bool success) {
        if (aliving) {
            if (success) {
                for (const auto &profile : *followers->profileList()) {
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
            }
            setRunning(false);
        }
        followers->deleteLater();
    });
    followers->setAccount(account());
    followers->getFollowers(targetDid(), 50, QString());
}
