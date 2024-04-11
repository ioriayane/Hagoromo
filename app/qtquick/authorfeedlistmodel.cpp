#include "authorfeedlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetauthorfeed.h"

using AtProtocolInterface::AppBskyFeedGetAuthorFeed;

AuthorFeedListModel::AuthorFeedListModel(QObject *parent) : TimelineListModel { parent }
{
    setDisplayInterval(0);
}

bool AuthorFeedListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetAuthorFeed *timeline = new AppBskyFeedGetAuthorFeed(this);
        connect(timeline, &AppBskyFeedGetAuthorFeed::finished, [=](bool success) {
            if (success) {
                if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                    m_cursor = timeline->cursor();
                }
                copyFrom(timeline->feedViewPostList());
            } else {
                emit errorOccured(timeline->errorCode(), timeline->errorMessage());
            }
            QTimer::singleShot(100, this, &AuthorFeedListModel::displayQueuedPosts);
            timeline->deleteLater();
        });

        QString filter_type;
        if (filter() == AuthorFeedListModelFilterType::PostsNoReplies) {
            filter_type = "posts_no_replies";
        } else if (filter() == AuthorFeedListModelFilterType::PostsWithMedia) {
            filter_type = "posts_with_media";
        } else if (filter() == AuthorFeedListModelFilterType::PostsAndAuthorThreads) {
            filter_type = "posts_and_author_threads";
        } else {
            filter_type = "posts_with_replies";
        }
        timeline->setAccount(account());
        timeline->setLabelers(m_contentFilterLabels.labelerDids());
        timeline->getAuthorFeed(authorDid(), -1, QString(), filter_type);
    });
    return true;
}

bool AuthorFeedListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetAuthorFeed *timeline = new AppBskyFeedGetAuthorFeed(this);
        connect(timeline, &AppBskyFeedGetAuthorFeed::finished, [=](bool success) {
            if (success) {
                m_cursor = timeline->cursor();
                copyFromNext(timeline->feedViewPostList());
            } else {
                emit errorOccured(timeline->errorCode(), timeline->errorMessage());
            }
            QTimer::singleShot(10, this, &AuthorFeedListModel::displayQueuedPostsNext);
            timeline->deleteLater();
        });

        QString filter_type;
        if (filter() == AuthorFeedListModelFilterType::PostsNoReplies) {
            filter_type = "posts_no_replies";
        } else if (filter() == AuthorFeedListModelFilterType::PostsWithMedia) {
            filter_type = "posts_with_media";
        } else if (filter() == AuthorFeedListModelFilterType::PostsAndAuthorThreads) {
            filter_type = "posts_and_author_threads";
        } else {
            filter_type = "posts_with_replies";
        }
        timeline->setAccount(account());
        timeline->setLabelers(m_contentFilterLabels.labelerDids());
        timeline->getAuthorFeed(authorDid(), -1, m_cursor, filter_type);
    });
    return true;
}

QString AuthorFeedListModel::authorDid() const
{
    return m_authorDid;
}

void AuthorFeedListModel::setAuthorDid(const QString &newAuthorDid)
{
    if (m_authorDid == newAuthorDid)
        return;
    m_authorDid = newAuthorDid;
    emit authorDidChanged();
}

AuthorFeedListModel::AuthorFeedListModelFilterType AuthorFeedListModel::filter() const
{
    return m_filter;
}

void AuthorFeedListModel::setFilter(AuthorFeedListModelFilterType newFilter)
{
    if (m_filter == newFilter)
        return;
    m_filter = newFilter;
    emit filterChanged();
}
