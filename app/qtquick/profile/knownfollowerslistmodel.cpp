#include "knownfollowerslistmodel.h"

#include "atprotocol/app/bsky/graph/appbskygraphgetknownfollowers.h"

using AtProtocolInterface::AppBskyGraphGetKnownFollowers;

KnownFollowersListModel::KnownFollowersListModel(QObject *parent) : FollowsListModel { parent } { }

bool KnownFollowersListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyGraphGetKnownFollowers *profiles = new AppBskyGraphGetKnownFollowers(this);
        connect(profiles, &AppBskyGraphGetKnownFollowers::finished, [=](bool success) {
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
        profiles->setLabelers(labelerDids());
        profiles->getKnownFollowers(targetDid(), 50, QString());
    });
    return true;
}

bool KnownFollowersListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyGraphGetKnownFollowers *profiles = new AppBskyGraphGetKnownFollowers(this);
        connect(profiles, &AppBskyGraphGetKnownFollowers::finished, [=](bool success) {
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
        profiles->setLabelers(labelerDids());
        profiles->getKnownFollowers(targetDid(), 50, m_cursor);
    });
    return true;
}
