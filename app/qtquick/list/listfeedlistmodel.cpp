#include "listfeedlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetlistfeed.h"

using AtProtocolInterface::AppBskyFeedGetListFeed;

ListFeedListModel::ListFeedListModel(QObject *parent) : TimelineListModel { parent } { }

bool ListFeedListModel::getLatest()
{
    if (running() || uri().isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetListFeed *list = new AppBskyFeedGetListFeed(this);
        connect(list, &AppBskyFeedGetListFeed::finished, [=](bool success) {
            if (success) {
                if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                    m_cursor = list->cursor();
                }
                copyFrom(list->feedList());
            } else {
                emit errorOccured(list->errorCode(), list->errorMessage());
            }
            QTimer::singleShot(100, this, &ListFeedListModel::displayQueuedPosts);
            list->deleteLater();
        });
        list->setAccount(account());
        list->setLabelers(labelerDids());
        list->getListFeed(uri(), 0, QString());
    });
    return true;
}

bool ListFeedListModel::getNext()
{
    if (running() || m_cursor.isEmpty() || uri().isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetListFeed *list = new AppBskyFeedGetListFeed(this);
        connect(list, &AppBskyFeedGetListFeed::finished, [=](bool success) {
            if (success) {
                m_cursor = list->cursor(); // 続きの読み込みの時は必ず上書き

                copyFromNext(list->feedList());
            } else {
                emit errorOccured(list->errorCode(), list->errorMessage());
            }
            QTimer::singleShot(10, this, &ListFeedListModel::displayQueuedPostsNext);
            list->deleteLater();
        });
        list->setAccount(account());
        list->setLabelers(labelerDids());
        list->getListFeed(uri(), 0, m_cursor);
    });
    return true;
}

QString ListFeedListModel::uri() const
{
    return m_uri;
}

void ListFeedListModel::setUri(const QString &newUri)
{
    if (m_uri == newUri)
        return;
    m_uri = newUri;
    emit uriChanged();
}
