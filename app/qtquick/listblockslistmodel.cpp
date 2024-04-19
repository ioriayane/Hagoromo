#include "listblockslistmodel.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetlistblocks.h"

using AtProtocolInterface::AppBskyGraphGetListBlocks;

ListBlocksListModel::ListBlocksListModel(QObject *parent) : ListsListModel { parent } { }

bool ListBlocksListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    AppBskyGraphGetListBlocks *lists = new AppBskyGraphGetListBlocks(this);
    connect(lists, &AppBskyGraphGetListBlocks::finished, [=](bool success) {
        if (success) {
            if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                m_cursor = lists->cursor();
            }
            copyFrom(lists);
            QTimer::singleShot(10, this, &ListBlocksListModel::displayQueuedPosts);
        } else {
            emit errorOccured(lists->errorCode(), lists->errorMessage());
            setRunning(false);
        }
        lists->deleteLater();
    });
    lists->setAccount(account());
    lists->getListBlocks(0, QString());

    return true;
}

bool ListBlocksListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    AppBskyGraphGetListBlocks *lists = new AppBskyGraphGetListBlocks(this);
    connect(lists, &AppBskyGraphGetListBlocks::finished, [=](bool success) {
        if (success) {
            m_cursor = lists->cursor(); // 続きの読み込みの時は必ず上書き
            if (lists->listViewList().isEmpty())
                m_cursor.clear(); // すべて読み切って空になったときはカーソルこないので空になるはずだけど念のため
            copyFrom(lists);
            QTimer::singleShot(10, this, &ListBlocksListModel::displayQueuedPostsNext);
        } else {
            m_cursor.clear();
            emit errorOccured(lists->errorCode(), lists->errorMessage());
            setRunning(false);
        }
        lists->deleteLater();
    });
    lists->setAccount(account());
    lists->getListBlocks(0, m_cursor);

    return true;
}
