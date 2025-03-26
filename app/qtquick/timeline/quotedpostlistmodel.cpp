#include "quotedpostlistmodel.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetquotes.h"

using AtProtocolInterface::AppBskyFeedGetQuotes;

QuotedPostListModel::QuotedPostListModel(QObject *parent) : TimelineListModel { parent } { }

bool QuotedPostListModel::getLatest()
{
    if (running() || postUri().isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetQuotes *posts = new AppBskyFeedGetQuotes(this);
        connect(posts, &AppBskyFeedGetQuotes::finished, [=](bool success) {
            if (success) {
                if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                    m_cursor = posts->cursor();
                }
                QList<AtProtocolType::AppBskyFeedDefs::FeedViewPost> feed_view_post_list;
                for (const auto &post : std::as_const(posts->postsList())) {
                    AtProtocolType::AppBskyFeedDefs::FeedViewPost feed_view_post;
                    feed_view_post.post = post;
                    feed_view_post_list.append(feed_view_post);
                }
                copyFrom(feed_view_post_list);
            } else {
                emit errorOccured(posts->errorCode(), posts->errorMessage());
            }
            QTimer::singleShot(100, this, &QuotedPostListModel::displayQueuedPosts);
            posts->deleteLater();
        });
        posts->setAccount(account());
        posts->setLabelers(labelerDids());
        posts->getQuotes(postUri(), QString(), 0, QString());
    });
    return true;
}

bool QuotedPostListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetQuotes *posts = new AppBskyFeedGetQuotes(this);
        connect(posts, &AppBskyFeedGetQuotes::finished, [=](bool success) {
            if (success) {
                m_cursor = posts->cursor();
                QList<AtProtocolType::AppBskyFeedDefs::FeedViewPost> feed_view_post_list;
                for (const auto &post : std::as_const(posts->postsList())) {
                    AtProtocolType::AppBskyFeedDefs::FeedViewPost feed_view_post;
                    feed_view_post.post = post;
                    feed_view_post_list.append(feed_view_post);
                }
                copyFromNext(feed_view_post_list);
            } else {
                emit errorOccured(posts->errorCode(), posts->errorMessage());
            }
            QTimer::singleShot(10, this, &QuotedPostListModel::displayQueuedPostsNext);
            posts->deleteLater();
        });

        posts->setAccount(account());
        posts->setLabelers(labelerDids());
        posts->getQuotes(postUri(), QString(), 0, m_cursor);
    });
    return true;
}

QString QuotedPostListModel::postUri() const
{
    return m_postUri;
}

void QuotedPostListModel::setPostUri(const QString &newPostUri)
{
    if (m_postUri == newPostUri)
        return;
    m_postUri = newPostUri;
    emit postUriChanged();
}

void QuotedPostListModel::finishedDisplayingQueuedPosts()
{
    setRunning(false);
}
