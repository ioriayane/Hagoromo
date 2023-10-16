#include "listfeedlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetlistfeed.h"

using AtProtocolInterface::AppBskyFeedGetListFeed;

ListFeedListModel::ListFeedListModel(QObject *parent) : TimelineListModel { parent } { }

void ListFeedListModel::getLatest()
{
    if (running() || uri().isEmpty())
        return;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetListFeed *list = new AppBskyFeedGetListFeed(this);
        connect(list, &AppBskyFeedGetListFeed::finished, [=](bool success) {
            if (success) {
                if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                    m_cursor = list->cursor();
                }
                copyFrom(list);
            } else {
                emit errorOccured(list->errorCode(), list->errorMessage());
            }
            QTimer::singleShot(100, this, &ListFeedListModel::displayQueuedPosts);
            list->deleteLater();
        });
        list->setAccount(account());
        list->getListFeed(uri(), 0, QString());
    });
}

void ListFeedListModel::getNext()
{
    if (running() || m_cursor.isEmpty() || uri().isEmpty())
        return;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetListFeed *list = new AppBskyFeedGetListFeed(this);
        connect(list, &AppBskyFeedGetListFeed::finished, [=](bool success) {
            if (success) {
                m_cursor = list->cursor(); // 続きの読み込みの時は必ず上書き

                copyFromNext(list);
            } else {
                emit errorOccured(list->errorCode(), list->errorMessage());
            }
            QTimer::singleShot(10, this, &ListFeedListModel::displayQueuedPostsNext);
            list->deleteLater();
        });
        list->setAccount(account());
        list->getListFeed(uri(), 0, m_cursor);
    });
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
