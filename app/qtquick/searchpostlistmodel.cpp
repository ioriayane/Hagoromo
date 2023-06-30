#include "searchpostlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"
#include "search/searchposts.h"

#include <QPointer>

using AtProtocolInterface::AppBskyFeedGetPosts;
using SearchInterface::SearchPosts;

SearchPostListModel::SearchPostListModel(QObject *parent) : TimelineListModel { parent }
{
    setDisplayInterval(0);
}

void SearchPostListModel::getLatest()
{
    if (running() || text().isEmpty())
        return;
    setRunning(true);

    QPointer<SearchPostListModel> aliving(this);

    SearchPosts *posts = new SearchPosts();
    connect(posts, &SearchPosts::finished, [=](bool success) {
        if (aliving) {
            if (success) {
                QDateTime reference_time = QDateTime::currentDateTimeUtc();

                for (const auto &view_post : *posts->viewPostList()) {
                    m_cueGetPost.append(
                            QString("at://%1/%2").arg(view_post.user.did, view_post.tid));

                    PostCueItem post;
                    post.cid = view_post.cid;
                    post.indexed_at = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
                    post.reference_time = reference_time;
                    m_cuePost.insert(0, post);
                }
            } else {
                emit errorOccured(posts->errorMessage());
            }
            QTimer::singleShot(100, this, &SearchPostListModel::displayQueuedPosts);
        }
        posts->deleteLater();
    });
    posts->setAccount(account());
    posts->setService(searchService());
    posts->search(text());
}

void SearchPostListModel::finishedDisplayingQueuedPosts()
{
    QTimer::singleShot(0, this, &SearchPostListModel::getPosts);
}

void SearchPostListModel::getPosts()
{
    if (m_cueGetPost.isEmpty()) {
        setRunning(false);
        return;
    }

    // getPostsは最大25個までいっきに取得できる
    QStringList uris;
    for (int i = 0; i < 25; i++) {
        if (m_cueGetPost.isEmpty())
            break;
        uris.append(m_cueGetPost.first());
        m_cueGetPost.removeFirst();
    }

    QPointer<SearchPostListModel> aliving(this);

    AppBskyFeedGetPosts *posts = new AppBskyFeedGetPosts();
    connect(posts, &AppBskyFeedGetPosts::finished, [=](bool success) {
        if (aliving) {
            if (success) {
                QStringList new_cid;

                for (auto item = posts->postList()->crbegin(); item != posts->postList()->crend();
                     item++) {
                    AtProtocolType::AppBskyFeedDefs::FeedViewPost view_post;
                    view_post.post = *item;
                    m_viewPostHash[item->cid] = view_post;

                    if (m_cidList.contains(item->cid)) {
                        int r = m_cidList.indexOf(item->cid);
                        if (r >= 0) {
                            emit dataChanged(index(r), index(r));
                        }
                    }
                }
            } else {
                emit errorOccured(posts->errorMessage());
            }
            // 残ってたらもう1回
            QTimer::singleShot(100, this, &SearchPostListModel::getPosts);
        }
        posts->deleteLater();
    });
    posts->setAccount(account());
    posts->getPosts(uris);
}

QString SearchPostListModel::text() const
{
    return m_text;
}

void SearchPostListModel::setText(const QString &newText)
{
    if (m_text == newText)
        return;
    m_text = newText;
    emit textChanged();
}

QString SearchPostListModel::searchService() const
{
    return m_searchService;
}

void SearchPostListModel::setSearchService(const QString &newSearchService)
{
    if (m_searchService == newSearchService)
        return;
    m_searchService = newSearchService;
    emit searchServiceChanged();
}
