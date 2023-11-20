#include "searchpostlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedsearchposts.h"

using AtProtocolInterface::AppBskyFeedSearchPosts;

SearchPostListModel::SearchPostListModel(QObject *parent) : TimelineListModel { parent } { }

bool SearchPostListModel::getLatest()
{
    if (running() || text().isEmpty())
        return false;
    setRunning(true);

    return updateContentFilterLabels([=]() {
        AppBskyFeedSearchPosts *posts = new AppBskyFeedSearchPosts(this);
        connect(posts, &AppBskyFeedSearchPosts::finished, [=](bool success) {
            if (success) {
                if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                    m_cursor = posts->cursor();
                }
                copyFrom(posts);
            } else {
                emit errorOccured(posts->errorCode(), posts->errorMessage());
            }
            QTimer::singleShot(100, this, &SearchPostListModel::displayQueuedPosts);
            posts->deleteLater();
        });
        posts->setAccount(account());
        if (!posts->searchPosts(text(), 10, QString())) {
            emit errorOccured(posts->errorCode(), posts->errorMessage());
            setRunning(false);
        }
    });
}

bool SearchPostListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    return updateContentFilterLabels([=]() {
        AppBskyFeedSearchPosts *posts = new AppBskyFeedSearchPosts(this);
        connect(posts, &AppBskyFeedSearchPosts::finished, [=](bool success) {
            if (success) {
                m_cursor = posts->cursor();
                copyFromNext(posts);
            } else {
                emit errorOccured(posts->errorCode(), posts->errorMessage());
            }
            QTimer::singleShot(10, this, &SearchPostListModel::displayQueuedPostsNext);
            posts->deleteLater();
        });

        posts->setAccount(account());
        if (!posts->searchPosts(text(), 10, m_cursor)) {
            emit errorOccured(posts->errorCode(), posts->errorMessage());
            setRunning(false);
        }
    });
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
