#include "listslistmodel.h"

#include "atprotocol/app/bsky/graph/appbskygraphgetlists.h"

using AtProtocolInterface::AppBskyGraphGetLists;
using namespace AtProtocolType;

ListsListModel::ListsListModel(QObject *parent) : AtpAbstractListModel { parent }
{
    setDisplayInterval(0);
}

int ListsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_cidList.count();
}

QVariant ListsListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<ListsListModelRoles>(role));
}

QVariant ListsListModel::item(int row, ListsListModelRoles role) const
{
    if (row < 0 || row >= m_cidList.count())
        return QVariant();

    const AppBskyGraphDefs::ListView &current = m_listViewHash.value(m_cidList.at(row));

    if (role == CidRole)
        return current.cid;
    else if (role == UriRole)
        return current.uri;
    else if (role == NameRole)
        return current.name;
    else if (role == DescriptionRole)
        return current.description;
    else if (role == CreatorHandleRole) {
        if (!current.creator)
            return QString();
        else
            return current.creator->handle;
    } else if (role == CreatorDisplayNameRole) {
        if (!current.creator)
            return QString();
        else
            return current.creator->displayName;
    } else if (role == CreatoravatarRole) {
        if (!current.creator)
            return QString();
        else
            return current.creator->avatar;
    }

    return QVariant();
}

int ListsListModel::indexOf(const QString &cid) const
{
    return m_cidList.indexOf(cid);
}

QString ListsListModel::getRecordText(const QString &cid)
{
    Q_UNUSED(cid)
    return QString();
}

void ListsListModel::clear()
{
    m_listViewHash.clear();
    AtpAbstractListModel::clear();
}

void ListsListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    AppBskyGraphGetLists *lists = new AppBskyGraphGetLists(this);
    connect(lists, &AppBskyGraphGetLists::finished, [=](bool success) {
        if (success) {
            if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                m_cursor = lists->cursor();
            }
            copyFrom(lists);
        } else {
            emit errorOccured(lists->errorCode(), lists->errorMessage());
        }
        QTimer::singleShot(10, this, &ListsListModel::displayQueuedPosts);
        lists->deleteLater();
    });
    lists->setAccount(account());
    lists->getLists(actor(), 0, QString());
}

void ListsListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return;
    setRunning(true);

    AppBskyGraphGetLists *lists = new AppBskyGraphGetLists(this);
    connect(lists, &AppBskyGraphGetLists::finished, [=](bool success) {
        if (success) {
            m_cursor = lists->cursor(); // 続きの読み込みの時は必ず上書き
            copyFrom(lists);
        } else {
            emit errorOccured(lists->errorCode(), lists->errorMessage());
        }
        QTimer::singleShot(10, this, &ListsListModel::displayQueuedPostsNext);
        lists->deleteLater();
    });
    lists->setAccount(account());
    lists->getLists(actor(), 0, m_cursor);
}

QHash<int, QByteArray> ListsListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[CidRole] = "cid";
    roles[UriRole] = "uri";
    roles[NameRole] = "name";
    roles[DescriptionRole] = "description";
    roles[CreatorHandleRole] = "creatorHandle";
    roles[CreatorDisplayNameRole] = "creatorDisplayName";
    roles[CreatoravatarRole] = "creatoravatar";

    return roles;
}

void ListsListModel::finishedDisplayingQueuedPosts()
{
    setRunning(false);
}

bool ListsListModel::checkVisibility(const QString &cid)
{
    if (visibilityType() == VisibilityTypeAll)
        return true;

    const AppBskyGraphDefs::ListView &current = m_listViewHash.value(cid);

    return (visibilityType() == VisibilityTypeCuration
            && current.purpose == "app.bsky.graph.defs#curatelist")
            || (visibilityType() == VisibilityTypeModeration
                && current.purpose == "app.bsky.graph.defs#modlist");
}

void ListsListModel::copyFrom(AtProtocolInterface::AppBskyGraphGetLists *lists)
{
    for (const auto &list : *lists->listViewList()) {
        m_listViewHash[list.cid] = list;

        PostCueItem post;
        post.cid = list.cid;
        post.indexed_at = list.indexedAt;
        post.reference_time = QDateTime::currentDateTimeUtc();
        m_cuePost.insert(0, post);
    }
}

QString ListsListModel::actor() const
{
    return m_actor;
}

void ListsListModel::setActor(const QString &newActor)
{
    if (m_actor == newActor)
        return;
    m_actor = newActor;
    emit actorChanged();
}

ListsListModel::VisibilityType ListsListModel::visibilityType() const
{
    return m_visibilityType;
}

void ListsListModel::setVisibilityType(const VisibilityType &newVisibilityType)
{
    if (m_visibilityType == newVisibilityType)
        return;
    m_visibilityType = newVisibilityType;
    emit visibilityTypeChanged();
}
