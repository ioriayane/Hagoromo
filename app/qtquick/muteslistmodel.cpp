#include "muteslistmodel.h"

#include "atprotocol/app/bsky/graph/appbskygraphgetmutes.h"

using AtProtocolInterface::AppBskyGraphGetMutes;

MutesListModel::MutesListModel(QObject *parent) : FollowsListModel { parent } { }

bool MutesListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyGraphGetMutes *profiles = new AppBskyGraphGetMutes(this);
        connect(profiles, &AppBskyGraphGetMutes::finished, [=](bool success) {
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
        profiles->getMutes(50, QString());
    });
    return true;
}

bool MutesListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyGraphGetMutes *profiles = new AppBskyGraphGetMutes(this);
        connect(profiles, &AppBskyGraphGetMutes::finished, [=](bool success) {
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
        profiles->getMutes(50, m_cursor);
    });
    return true;
}
