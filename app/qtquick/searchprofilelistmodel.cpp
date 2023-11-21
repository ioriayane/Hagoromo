#include "searchprofilelistmodel.h"

#include "atprotocol/app/bsky/actor/appbskyactorsearchactors.h"

using AtProtocolInterface::AppBskyActorSearchActors;

SearchProfileListModel::SearchProfileListModel(QObject *parent) : FollowsListModel { parent } { }

bool SearchProfileListModel::getLatest()
{
    if (running() || text().isEmpty())
        return false;
    setRunning(true);

    return updateContentFilterLabels([=]() {
        AppBskyActorSearchActors *profiles = new AppBskyActorSearchActors(this);
        connect(profiles, &AppBskyActorSearchActors::finished, [=](bool success) {
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
        if (!profiles->searchActors(text(), 10, QString())) {
            emit errorOccured(profiles->errorCode(), profiles->errorMessage());
            setRunning(false);
        }
    });
}

bool SearchProfileListModel::getNext()
{
    return updateContentFilterLabels([=]() {
        AppBskyActorSearchActors *profiles = new AppBskyActorSearchActors(this);
        connect(profiles, &AppBskyActorSearchActors::finished, [=](bool success) {
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
        if (!profiles->searchActors(text(), 10, m_cursor)) {
            emit errorOccured(profiles->errorCode(), profiles->errorMessage());
            setRunning(false);
        }
    });
}

QString SearchProfileListModel::text() const
{
    return m_text;
}

void SearchProfileListModel::setText(const QString &newText)
{
    if (m_text == newText)
        return;
    m_text = newText;
    emit textChanged();
}

QString SearchProfileListModel::searchService() const
{
    return m_searchService;
}

void SearchProfileListModel::setSearchService(const QString &newSearchService)
{
    if (m_searchService == newSearchService)
        return;
    m_searchService = newSearchService;
    emit searchServiceChanged();
}
