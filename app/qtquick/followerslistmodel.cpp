#include "followerslistmodel.h"

#include "atprotocol/app/bsky/graph/appbskygraphgetfollowers.h"

using AtProtocolInterface::AppBskyGraphGetFollowers;

FollowersListModel::FollowersListModel(QObject *parent) : FollowsListModel { parent } { }

void FollowersListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyGraphGetFollowers *followers = new AppBskyGraphGetFollowers(this);
        connect(followers, &AppBskyGraphGetFollowers::finished, [=](bool success) {
            if (success) {
                if (m_didList.isEmpty()) {
                    m_cursor = followers->cursor();
                }
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
    });
}

void FollowersListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyGraphGetFollowers *followers = new AppBskyGraphGetFollowers(this);
        connect(followers, &AppBskyGraphGetFollowers::finished, [=](bool success) {
            if (success) {
                m_cursor = followers->cursor();
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
        followers->getFollowers(targetDid(), 50, m_cursor);
    });
}
