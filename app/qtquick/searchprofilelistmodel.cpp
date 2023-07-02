#include "searchprofilelistmodel.h"

#include "search/searchprofiles.h"

#include <QPointer>

using SearchInterface::SearchProfiles;

SearchProfileListModel::SearchProfileListModel(QObject *parent) : FollowsListModel { parent } { }

void SearchProfileListModel::getLatest()
{
    if (running() || text().isEmpty())
        return;
    setRunning(true);

    QPointer<SearchProfileListModel> aliving(this);

    SearchProfiles *profiles = new SearchProfiles();
    connect(profiles, &SearchProfiles::finished, [=](bool success) {
        if (aliving) {
            if (success) {
                m_cueGetProfile.append(*profiles->didList());
            } else {
                emit errorOccured(profiles->errorMessage());
            }
            QTimer::singleShot(100, this, &SearchProfileListModel::getProfiles);
        }
        profiles->deleteLater();
    });
    profiles->setAccount(account());
    profiles->setService(searchService());
    profiles->search(text());
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
