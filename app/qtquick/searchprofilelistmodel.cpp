#include "searchprofilelistmodel.h"

#include "atprotocol/app/bsky/actor/appbskyactorgetprofiles.h"
#include "search/searchprofiles.h"

#include <QPointer>

using AtProtocolInterface::AppBskyActorGetProfiles;
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
            }
            QTimer::singleShot(100, this, &SearchProfileListModel::getProfiles);
        }
        profiles->deleteLater();
    });
    profiles->setAccount(account());
    profiles->setService(searchService());
    profiles->search(text());
}

void SearchProfileListModel::getProfiles()
{
    if (m_cueGetProfile.isEmpty()) {
        setRunning(false);
        return;
    }

    // getProfilesは最大25個までいっきに取得できる
    QStringList dids;
    for (int i = 0; i < 25; i++) {
        if (m_cueGetProfile.isEmpty())
            break;
        dids.append(m_cueGetProfile.first());
        m_cueGetProfile.removeFirst();
    }

    QPointer<SearchProfileListModel> aliving(this);

    AppBskyActorGetProfiles *posts = new AppBskyActorGetProfiles();
    connect(posts, &AppBskyActorGetProfiles::finished, [=](bool success) {
        if (aliving) {
            if (success) {
                QStringList new_cid;

                for (auto item = posts->profileViewDetaileds()->crbegin();
                     item != posts->profileViewDetaileds()->crend(); item++) {
                    AtProtocolType::AppBskyActorDefs::ProfileView profile_view;
                    profile_view.avatar = item->avatar;
                    profile_view.did = item->did;
                    profile_view.displayName = item->displayName;
                    profile_view.handle = item->handle;
                    profile_view.description = item->description;
                    profile_view.indexedAt = item->indexedAt;
                    profile_view.labels = item->labels;
                    profile_view.viewer = item->viewer;
                    m_profileHash[item->did] = profile_view;
                    if (m_didList.contains(item->did)) {
                        int r = m_didList.indexOf(item->did);
                        if (r >= 0) {
                            emit dataChanged(index(r), index(r));
                        }
                    } else {
                        beginInsertRows(QModelIndex(), m_didList.count(), m_didList.count());
                        m_didList.append(item->did);
                        endInsertRows();
                    }
                }
            }
            // 残ってたらもう1回
            QTimer::singleShot(100, this, &SearchProfileListModel::getProfiles);
        }
        posts->deleteLater();
    });
    posts->setAccount(account());
    posts->getProfiles(dids);
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
