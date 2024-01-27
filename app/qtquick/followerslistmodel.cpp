#include "followerslistmodel.h"

#include "atprotocol/app/bsky/graph/appbskygraphgetfollowers.h"

using AtProtocolInterface::AppBskyGraphGetFollowers;

FollowersListModel::FollowersListModel(QObject *parent) : FollowsListModel { parent } { }

bool FollowersListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyGraphGetFollowers *profiles = new AppBskyGraphGetFollowers(this);
        connect(profiles, &AppBskyGraphGetFollowers::finished, [=](bool success) {
            if (success) {
                if (m_didList.isEmpty()) {
                    m_cursor = profiles->cursor();
                }
                copyProfiles(profiles);
            } else {
                emit errorOccured(profiles->errorCode(), profiles->errorMessage());
            }
            setRunning(false);
            profiles->deleteLater();
        });
        profiles->setAccount(account());
        profiles->getFollowers(targetDid(), 50, QString());
    });
    return true;
}

bool FollowersListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyGraphGetFollowers *profiles = new AppBskyGraphGetFollowers(this);
        connect(profiles, &AppBskyGraphGetFollowers::finished, [=](bool success) {
            if (success) {
                m_cursor = profiles->cursor();
                copyProfiles(profiles);
            } else {
                emit errorOccured(profiles->errorCode(), profiles->errorMessage());
            }
            setRunning(false);
            profiles->deleteLater();
        });
        profiles->setAccount(account());
        profiles->getFollowers(targetDid(), 50, m_cursor);
    });
    return true;
}
