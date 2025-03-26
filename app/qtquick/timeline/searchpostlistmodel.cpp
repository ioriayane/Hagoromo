#include "searchpostlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedsearchposts.h"

using AtProtocolInterface::AppBskyFeedSearchPosts;

SearchPostListModel::SearchPostListModel(QObject *parent) : TimelineListModel { parent } { }

bool SearchPostListModel::getLatest()
{
    if (running() || text().isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedSearchPosts *posts = new AppBskyFeedSearchPosts(this);
        connect(posts, &AppBskyFeedSearchPosts::finished, [=](bool success) {
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
            QTimer::singleShot(100, this, &SearchPostListModel::displayQueuedPosts);
            posts->deleteLater();
        });
        posts->setAccount(account());
        posts->setLabelers(labelerDids());
        posts->searchPosts(replaceSearchCommand(text()), QString(), QString(), QString(), QString(),
                           QString(), QString(), QString(), QString(), QStringList(), 10,
                           QString());
    });
    return true;
}

bool SearchPostListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedSearchPosts *posts = new AppBskyFeedSearchPosts(this);
        connect(posts, &AppBskyFeedSearchPosts::finished, [=](bool success) {
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
            QTimer::singleShot(10, this, &SearchPostListModel::displayQueuedPostsNext);
            posts->deleteLater();
        });

        posts->setAccount(account());
        posts->setLabelers(labelerDids());
        posts->searchPosts(replaceSearchCommand(text()), QString(), QString(), QString(), QString(),
                           QString(), QString(), QString(), QString(), QStringList(), 10, m_cursor);
    });
    return true;
}

QString SearchPostListModel::replaceSearchCommand(const QString &command)
{
    QStringList items = command.split(QRegularExpression(QString("[ \t%1]").arg(QChar(0x3000))));
    QStringList replaced_items;
    for (const auto &item : std::as_const(items)) {
        if (item == "from:me") {
            replaced_items.append("from:" + account().handle);
        } else {
            replaced_items.append(item);
        }
    }
    return replaced_items.join(' ');
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
