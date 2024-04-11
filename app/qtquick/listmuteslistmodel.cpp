#include "listmuteslistmodel.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetlistmutes.h"

using AtProtocolInterface::AppBskyGraphGetListMutes;

ListMutesListModel::ListMutesListModel(QObject *parent) : ListsListModel { parent } { }

bool ListMutesListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    AppBskyGraphGetListMutes *lists = new AppBskyGraphGetListMutes(this);
    connect(lists, &AppBskyGraphGetListMutes::finished, [=](bool success) {
        if (success) {
            if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                m_cursor = lists->cursor();
            }
            copyFrom(lists);
            QTimer::singleShot(10, this, &ListMutesListModel::displayQueuedPosts);
        } else {
            emit errorOccured(lists->errorCode(), lists->errorMessage());
            setRunning(false);
        }
        lists->deleteLater();
    });
    lists->setAccount(account());
    lists->getListMutes(0, QString());

    return true;
}

bool ListMutesListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    AppBskyGraphGetListMutes *lists = new AppBskyGraphGetListMutes(this);
    connect(lists, &AppBskyGraphGetListMutes::finished, [=](bool success) {
        if (success) {
            m_cursor = lists->cursor(); // 続きの読み込みの時は必ず上書き
            if (lists->listViewList().isEmpty())
                m_cursor.clear(); // すべて読み切って空になったときはカーソルこないので空になるはずだけど念のため
            copyFrom(lists);
            QTimer::singleShot(10, this, &ListMutesListModel::displayQueuedPostsNext);
        } else {
            m_cursor.clear();
            emit errorOccured(lists->errorCode(), lists->errorMessage());
            setRunning(false);
        }
        lists->deleteLater();
    });
    lists->setAccount(account());
    lists->getListMutes(0, m_cursor);

    return true;
}
