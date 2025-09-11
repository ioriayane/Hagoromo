#include "bookmarkpostlistmodel.h"
#include "atprotocol/app/bsky/bookmark/appbskybookmarkgetbookmarks.h"

using AtProtocolInterface::AppBskyBookmarkGetBookmarks;

BookmarkPostListModel::BookmarkPostListModel(QObject *parent) : TimelineListModel { parent } { }

bool BookmarkPostListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        auto bookmarks = new AppBskyBookmarkGetBookmarks(this);
        connect(bookmarks, &AppBskyBookmarkGetBookmarks::finished, [=](bool success) {
            if (success) {
                if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                    m_cursor = bookmarks->cursor();
                }
                QList<AtProtocolType::AppBskyFeedDefs::FeedViewPost> feed_view_post_list;
                for (const auto &bookmark : std::as_const(bookmarks->bookmarksList())) {
                    AtProtocolType::AppBskyFeedDefs::FeedViewPost feed_view_post;
                    feed_view_post.post = bookmark.item_AppBskyFeedDefs_PostView;
                    feed_view_post_list.append(feed_view_post);
                }
                copyFrom(feed_view_post_list);
            } else {
                emit errorOccured(bookmarks->errorCode(), bookmarks->errorMessage());
            }
            QTimer::singleShot(0, this, &BookmarkPostListModel::displayQueuedPosts);
            bookmarks->deleteLater();
        });
        bookmarks->setAccount(account());
        bookmarks->setLabelers(labelerDids());
        bookmarks->getBookmarks(0, QString());
    });
    return true;
}

bool BookmarkPostListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        auto bookmarks = new AppBskyBookmarkGetBookmarks(this);
        connect(bookmarks, &AppBskyBookmarkGetBookmarks::finished, [=](bool success) {
            if (success) {
                m_cursor = bookmarks->cursor();
                QList<AtProtocolType::AppBskyFeedDefs::FeedViewPost> feed_view_post_list;
                for (const auto &bookmark : std::as_const(bookmarks->bookmarksList())) {
                    AtProtocolType::AppBskyFeedDefs::FeedViewPost feed_view_post;
                    feed_view_post.post = bookmark.item_AppBskyFeedDefs_PostView;
                    feed_view_post_list.append(feed_view_post);
                }
                copyFromNext(feed_view_post_list);
            } else {
                emit errorOccured(bookmarks->errorCode(), bookmarks->errorMessage());
            }
            QTimer::singleShot(0, this, &BookmarkPostListModel::displayQueuedPostsNext);
            bookmarks->deleteLater();
        });
        bookmarks->setAccount(account());
        bookmarks->setLabelers(labelerDids());
        bookmarks->getBookmarks(0, m_cursor);
    });
    return true;
}

void BookmarkPostListModel::finishedDisplayingQueuedPosts()
{
    setRunning(false);
}
