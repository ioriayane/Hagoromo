#include "listslistmodel.h"
#include "listitemlistmodel.h"

#include "atprotocol/app/bsky/graph/appbskygraphgetlists.h"

using AtProtocolInterface::AppBskyGraphGetLists;
using namespace AtProtocolType;

ListsListModel::ListsListModel(QObject *parent)
    : AtpAbstractListModel { parent }, m_visibilityType(ListsListModel::VisibilityTypeAll)
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
    else if (role == AvatarRole)
        return current.avatar;
    else if (role == NameRole)
        return current.name;
    else if (role == DescriptionRole)
        return current.description;
    else if (role == SubscribedRole)
        return current.viewer.muted;
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
    } else if (role == SearchStatusRole)
        return SearchStatusTypeUnknown;

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

bool ListsListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    m_searchQue.clear();
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

    return true;
}

bool ListsListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    m_searchQue.clear();
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

    return true;
}

QHash<int, QByteArray> ListsListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[CidRole] = "cid";
    roles[UriRole] = "uri";
    roles[AvatarRole] = "avatar";
    roles[NameRole] = "name";
    roles[DescriptionRole] = "description";
    roles[SubscribedRole] = "subscribed";
    roles[CreatorHandleRole] = "creatorHandle";
    roles[CreatorDisplayNameRole] = "creatorDisplayName";
    roles[CreatoravatarRole] = "creatoravatar";
    roles[SearchStatusRole] = "searchStatus";

    return roles;
}

void ListsListModel::finishedDisplayingQueuedPosts()
{
    if (m_searchQue.isEmpty()) {
        setRunning(false);
    } else {
        QTimer::singleShot(0, this, &ListsListModel::searchActorInEachLists);
    }
}

bool ListsListModel::checkVisibility(const QString &cid)
{
    if (visibilityType() == VisibilityTypeAll)
        return true;

    const AppBskyGraphDefs::ListView &current = m_listViewHash.value(cid);

    if ((visibilityType() == VisibilityTypeCuration
         && current.purpose == "app.bsky.graph.defs#curatelist")
        || (visibilityType() == VisibilityTypeModeration
            && current.purpose == "app.bsky.graph.defs#modlist")) {
        if (searchTarget().startsWith("did:")) {
            m_searchQue.append(cid);
        }
        return true;
    } else {
        return false;
    }
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

void ListsListModel::searchActorInEachLists()
{
    if (m_searchQue.isEmpty()) {
        setRunning(false);
        return;
    }

    ListItemListModel *list = new ListItemListModel(this);
    m_searchQue.pop_back();
    QTimer::singleShot(0, this, &ListsListModel::searchActorInEachLists);
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

QString ListsListModel::searchTarget() const
{
    return m_searchTarget;
}

void ListsListModel::setSearchTarget(const QString &newSearchTarget)
{
    if (m_searchTarget == newSearchTarget)
        return;
    m_searchTarget = newSearchTarget;
    emit searchTargetChanged();
}
