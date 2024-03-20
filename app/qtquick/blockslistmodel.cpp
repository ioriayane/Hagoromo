#include "blockslistmodel.h"

#include "atprotocol/app/bsky/graph/appbskygraphgetblocks.h"

using AtProtocolInterface::AppBskyGraphGetBlocks;

BlocksListModel::BlocksListModel(QObject *parent) : FollowsListModel { parent } { }

bool BlocksListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyGraphGetBlocks *profiles = new AppBskyGraphGetBlocks(this);
        connect(profiles, &AppBskyGraphGetBlocks::finished, [=](bool success) {
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
        profiles->setLabelers(m_contentFilterLabels.labelerDids());
        profiles->getBlocks(50, QString());
    });
    return true;
}

bool BlocksListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyGraphGetBlocks *profiles = new AppBskyGraphGetBlocks(this);
        connect(profiles, &AppBskyGraphGetBlocks::finished, [=](bool success) {
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
        profiles->setLabelers(m_contentFilterLabels.labelerDids());
        profiles->getBlocks(50, m_cursor);
    });
    return true;
}
