#include "followerslistmodel.h"

#include "atprotocol/app/bsky/graph/appbskygraphgetfollowers.h"

using AtProtocolInterface::AppBskyGraphGetFollowers;

FollowersListModel::FollowersListModel(QObject *parent) : FollowsListModel { parent } { }

void FollowersListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    AppBskyGraphGetFollowers *followers = new AppBskyGraphGetFollowers(this);
    connect(followers, &AppBskyGraphGetFollowers::finished, [=](bool success) {
        if (success) {
            for (const auto &profile : *followers->profileList()) {
                m_profileHash[profile.did] = profile;
                m_formattedDescriptionHash[profile.did] =
                        m_systemTool.markupText(profile.description);
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
            emit errorOccured(followers->errorMessage());
        }
        setRunning(false);
        followers->deleteLater();
    });
    followers->setAccount(account());
    followers->getFollowers(targetDid(), 50, QString());
}
