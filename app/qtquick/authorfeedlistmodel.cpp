#include "authorfeedlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetauthorfeed.h"

using AtProtocolInterface::AppBskyFeedGetAuthorFeed;

AuthorFeedListModel::AuthorFeedListModel(QObject *parent) : TimelineListModel { parent }
{
    setDisplayInterval(0);
}

void AuthorFeedListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetAuthorFeed *timeline = new AppBskyFeedGetAuthorFeed(this);
        connect(timeline, &AppBskyFeedGetAuthorFeed::finished, [=](bool success) {
            if (success) {
                if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                    m_cursor = timeline->cursor();
                }
                copyFrom(timeline);
            } else {
                emit errorOccured(timeline->errorCode(), timeline->errorMessage());
            }
            QTimer::singleShot(100, this, &AuthorFeedListModel::displayQueuedPosts);
            timeline->deleteLater();
        });

        AppBskyFeedGetAuthorFeed::FilterType filter_type;
        if (filter() == AuthorFeedListModelFilterType::PostsNoReplies) {
            filter_type = AppBskyFeedGetAuthorFeed::FilterType::PostsNoReplies;
        } else if (filter() == AuthorFeedListModelFilterType::PostsWithMedia) {
            filter_type = AppBskyFeedGetAuthorFeed::FilterType::PostsWithMedia;
        } else {
            filter_type = AppBskyFeedGetAuthorFeed::FilterType::PostsWithReplies;
        }
        timeline->setAccount(account());
        timeline->getAuthorFeed(authorDid(), -1, QString(), filter_type);
    });
}

void AuthorFeedListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetAuthorFeed *timeline = new AppBskyFeedGetAuthorFeed(this);
        connect(timeline, &AppBskyFeedGetAuthorFeed::finished, [=](bool success) {
            if (success) {
                m_cursor = timeline->cursor();
                copyFromNext(timeline);
            } else {
                emit errorOccured(timeline->errorCode(), timeline->errorMessage());
            }
            QTimer::singleShot(10, this, &AuthorFeedListModel::displayQueuedPostsNext);
            timeline->deleteLater();
        });

        AppBskyFeedGetAuthorFeed::FilterType filter_type;
        if (filter() == AuthorFeedListModelFilterType::PostsNoReplies) {
            filter_type = AppBskyFeedGetAuthorFeed::FilterType::PostsNoReplies;
        } else if (filter() == AuthorFeedListModelFilterType::PostsWithMedia) {
            filter_type = AppBskyFeedGetAuthorFeed::FilterType::PostsWithMedia;
        } else {
            filter_type = AppBskyFeedGetAuthorFeed::FilterType::PostsWithReplies;
        }
        timeline->setAccount(account());
        timeline->getAuthorFeed(authorDid(), -1, m_cursor, filter_type);
    });
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
