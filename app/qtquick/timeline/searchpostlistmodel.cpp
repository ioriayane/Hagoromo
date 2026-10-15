#include "searchpostlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedsearchposts.h"
#include "atprotocol/app/bsky/feed/appbskyfeedsearchpostsv2.h"

using AtProtocolInterface::AppBskyFeedSearchPosts;
using AtProtocolInterface::AppBskyFeedSearchPostsV2;

SearchPostListModel::SearchPostListModel(QObject *parent)
    : TimelineListModel { parent }, m_useSearchPostsV2(true)
{
}

bool SearchPostListModel::getLatest()
{
    if (running() || text().isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        if (m_useSearchPostsV2) {
            searchPostsV2Latest();
        } else {
            searchPostsLatest();
        }
    });
    return true;
}

bool SearchPostListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        if (m_useSearchPostsV2) {
            searchPostsV2Next();
        } else {
            searchPostsNext();
        }
    });
    return true;
}

void SearchPostListModel::searchPostsV2Latest()
{
    AppBskyFeedSearchPostsV2 *posts = new AppBskyFeedSearchPostsV2(this);
    connect(posts, &AppBskyFeedSearchPostsV2::finished, this, [=](bool success) {
        if (success) {
            if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                m_cursor = posts->cursor();
            }
            appendPosts(posts->postsList(), false);
            QTimer::singleShot(100, this, &SearchPostListModel::displayQueuedPosts);
        } else {
            // 同じ条件でsearchPostsで取得し直す
            switchToSearchPosts();
            searchPostsLatest();
        }
        posts->deleteLater();
    });
    const SearchPostsV2Condition condition = makeSearchPostsV2Condition(text());
    posts->setAccount(account());
    posts->setLabelers(labelerDids());
    posts->searchPostsV2(QString(), 10, condition.query, "recent", QStringList(), QStringList(),
                         QStringList(), QStringList(), QStringList(), QStringList(), QStringList(),
                         QStringList(), QStringList(), QStringList(), QStringList(), QStringList(),
                         condition.since, condition.until, true, condition.languages, QStringList(),
                         false, false, QString(), QString(), false, false, false, QString());
}

void SearchPostListModel::searchPostsV2Next()
{
    AppBskyFeedSearchPostsV2 *posts = new AppBskyFeedSearchPostsV2(this);
    connect(posts, &AppBskyFeedSearchPostsV2::finished, this, [=](bool success) {
        if (success) {
            m_cursor = posts->cursor();
            appendPosts(posts->postsList(), true);
        } else {
            // searchPostsV2のカーソルはsearchPostsで使えないので続きの読み込みはここで終える
            switchToSearchPosts();
        }
        QTimer::singleShot(10, this, &SearchPostListModel::displayQueuedPostsNext);
        posts->deleteLater();
    });
    const SearchPostsV2Condition condition = makeSearchPostsV2Condition(text());
    posts->setAccount(account());
    posts->setLabelers(labelerDids());
    posts->searchPostsV2(m_cursor, 10, condition.query, "recent", QStringList(), QStringList(),
                         QStringList(), QStringList(), QStringList(), QStringList(), QStringList(),
                         QStringList(), QStringList(), QStringList(), QStringList(), QStringList(),
                         condition.since, condition.until, true, condition.languages, QStringList(),
                         false, false, QString(), QString(), false, false, false, QString());
}

void SearchPostListModel::searchPostsLatest()
{
    AppBskyFeedSearchPosts *posts = new AppBskyFeedSearchPosts(this);
    connect(posts, &AppBskyFeedSearchPosts::finished, this, [=](bool success) {
        if (success) {
            if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                m_cursor = posts->cursor();
            }
            appendPosts(posts->postsList(), false);
        } else {
            emit errorOccurred(posts->errorCode(), posts->errorMessage());
        }
        QTimer::singleShot(100, this, &SearchPostListModel::displayQueuedPosts);
        posts->deleteLater();
    });
    posts->setAccount(account());
    posts->setLabelers(labelerDids());
    posts->searchPosts(replaceSearchCommand(text()), QString(), QString(), QString(), QString(),
                       QString(), QString(), QString(), QString(), QStringList(), 10, QString());
}

void SearchPostListModel::searchPostsNext()
{
    AppBskyFeedSearchPosts *posts = new AppBskyFeedSearchPosts(this);
    connect(posts, &AppBskyFeedSearchPosts::finished, this, [=](bool success) {
        if (success) {
            m_cursor = posts->cursor();
            appendPosts(posts->postsList(), true);
        } else {
            emit errorOccurred(posts->errorCode(), posts->errorMessage());
        }
        QTimer::singleShot(10, this, &SearchPostListModel::displayQueuedPostsNext);
        posts->deleteLater();
    });
    posts->setAccount(account());
    posts->setLabelers(labelerDids());
    posts->searchPosts(replaceSearchCommand(text()), QString(), QString(), QString(), QString(),
                       QString(), QString(), QString(), QString(), QStringList(), 10, m_cursor);
}

void SearchPostListModel::switchToSearchPosts()
{
    qDebug().noquote() << "Switch to app.bsky.feed.searchPosts";
    m_useSearchPostsV2 = false;
    m_cursor.clear();
}

void SearchPostListModel::appendPosts(const QList<AtProtocolType::AppBskyFeedDefs::PostView> &posts,
                                      const bool next)
{
    QList<AtProtocolType::AppBskyFeedDefs::FeedViewPost> feed_view_post_list;
    for (const auto &post : posts) {
        AtProtocolType::AppBskyFeedDefs::FeedViewPost feed_view_post;
        feed_view_post.post = post;
        feed_view_post_list.append(feed_view_post);
    }
    if (next) {
        copyFromNext(feed_view_post_list);
    } else {
        copyFrom(feed_view_post_list);
    }
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

SearchPostsV2Condition SearchPostListModel::makeSearchPostsV2Condition(const QString &command)
{
    // searchPostsV2はqueryに含まれるsince:/until:/lang:を解釈しないのでパラメータに移す
    SearchPostsV2Condition condition;
    QStringList items = replaceSearchCommand(command).split(' ');
    QStringList query_items;
    for (const auto &item : std::as_const(items)) {
        if (item.startsWith("since:") && item.length() > 6) {
            condition.since = item.mid(6);
        } else if (item.startsWith("until:") && item.length() > 6) {
            condition.until = item.mid(6);
        } else if (item.startsWith("lang:") && item.length() > 5) {
            condition.languages.append(item.mid(5));
        } else {
            query_items.append(item);
        }
    }
    condition.query = query_items.join(' ').trimmed();
    return condition;
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
