#include "followerslistmodel.h"

#include "atprotocol/app/bsky/graph/appbskygraphgetfollowers.h"

using AtProtocolInterface::AppBskyGraphGetFollowers;

FollowersListModel::FollowersListModel(QObject *parent) : FollowsListModel { parent } { }

bool FollowersListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    return updateContentFilterLabels([=]() {
        AppBskyGraphGetFollowers *followers = new AppBskyGraphGetFollowers(this);
        connect(followers, &AppBskyGraphGetFollowers::finished, [=](bool success) {
            if (success) {
                if (m_didList.isEmpty()) {
                    m_cursor = followers->cursor();
                }
                copyProfiles(followers);
            } else {
                emit errorOccured(followers->errorCode(), followers->errorMessage());
            }
            setRunning(false);
            followers->deleteLater();
        });
        followers->setAccount(account());
        followers->getFollowers(targetDid(), 50, QString());
    });
}

bool FollowersListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    return updateContentFilterLabels([=]() {
        AppBskyGraphGetFollowers *followers = new AppBskyGraphGetFollowers(this);
        connect(followers, &AppBskyGraphGetFollowers::finished, [=](bool success) {
            if (success) {
                m_cursor = followers->cursor();
                copyProfiles(followers);
            } else {
                emit errorOccured(followers->errorCode(), followers->errorMessage());
            }
            setRunning(false);
            followers->deleteLater();
        });
        followers->setAccount(account());
        followers->getFollowers(targetDid(), 50, m_cursor);
    });
}
