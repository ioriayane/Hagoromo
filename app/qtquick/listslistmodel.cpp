#include "listslistmodel.h"
#include "listitemlistmodel.h"
#include "recordoperator.h"

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
    } else if (role == SearchStatusRole) {
        if (m_searchStatusHash.contains(current.cid)) {
            return m_searchStatusHash[current.cid];
        } else {
            return SearchStatusTypeUnknown;
        }
    } else if (role == ListItemUriRole) {
        if (m_listItemUriHash.contains(current.cid)) {
            return m_listItemUriHash[current.cid];
        } else {
            return QString();
        }
    }

    return QVariant();
}

void ListsListModel::update(int row, ListsListModelRoles role, const QVariant &value)
{
    if (row < 0 || row >= m_cidList.count())
        return;

    const AppBskyGraphDefs::ListView &current = m_listViewHash.value(m_cidList.at(row));

    if (role == SearchStatusRole) {
        SearchStatusType new_type = static_cast<SearchStatusType>(value.toInt());
        if (!m_searchStatusHash.contains(current.cid)
            || m_searchStatusHash[current.cid] != new_type) {
            m_searchStatusHash[current.cid] = new_type;
            emit dataChanged(index(row), index(row));
        }
    } else if (role == ListItemUriRole) {
        m_listItemUriHash[current.cid] = value.toString();
        emit dataChanged(index(row), index(row));
    }
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
    m_searchStatusHash.clear();
    m_listItemUriHash.clear();
    AtpAbstractListModel::clear();
}

bool ListsListModel::addRemoveFromList(const int row, const QString &did)
{
    QString uri = item(row, ListsListModel::UriRole).toString();
    SearchStatusType status =
            static_cast<SearchStatusType>(item(row, ListsListModel::SearchStatusRole).toInt());
    if (status == SearchStatusTypeContains) {
        uri = ""; // change to ListItem uri
    }
    if (running() || uri.isEmpty())
        return false;

    RecordOperator *ope = new RecordOperator(this);
    connect(ope, &RecordOperator::finished,
            [=](bool success, const QString &uri, const QString &cid) {
                Q_UNUSED(uri)
                Q_UNUSED(cid)
                if (success) {
                    update(row, ListsListModel::SearchStatusRole,
                           SearchStatusType::SearchStatusTypeContains);
                }
                setRunning(false);
                ope->deleteLater();
            });
    ope->setAccount(account().service, account().did, account().handle, account().email,
                    account().accessJwt, account().refreshJwt);
    if (status == SearchStatusTypeContains) {
        // delete
        setRunning(ope->deleteListItem(uri));
    } else if (status == SearchStatusTypeNotContains) {
        // Add
        setRunning(ope->listItem(uri, did));
    } else {
        ope->deleteLater();
    }
    return running();
}

bool ListsListModel::getLatest()
{
    if (running())
        return false;

    m_searchCidQue.clear();
    AppBskyGraphGetLists *lists = new AppBskyGraphGetLists(this);
    connect(lists, &AppBskyGraphGetLists::finished, [=](bool success) {
        if (success) {
            if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                m_cursor = lists->cursor();
            }
            copyFrom(lists);
            QTimer::singleShot(10, this, &ListsListModel::displayQueuedPosts);
        } else {
            emit errorOccured(lists->errorCode(), lists->errorMessage());
            setRunning(false);
        }
        lists->deleteLater();
    });
    lists->setAccount(account());
    setRunning(lists->getLists(actor(), 0, QString()));

    return running();
}

bool ListsListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;

    m_searchCidQue.clear();
    AppBskyGraphGetLists *lists = new AppBskyGraphGetLists(this);
    connect(lists, &AppBskyGraphGetLists::finished, [=](bool success) {
        if (success) {
            m_cursor = lists->cursor(); // 続きの読み込みの時は必ず上書き
            copyFrom(lists);
            QTimer::singleShot(10, this, &ListsListModel::displayQueuedPostsNext);
        } else {
            emit errorOccured(lists->errorCode(), lists->errorMessage());
            setRunning(false);
        }
        lists->deleteLater();
    });
    lists->setAccount(account());
    setRunning(lists->getLists(actor(), 0, m_cursor));

    return running();
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
    roles[ListItemUriRole] = "listItemUri";

    return roles;
}

void ListsListModel::finishedDisplayingQueuedPosts()
{
    if (m_searchCidQue.isEmpty()) {
        setRunning(false);
    } else {
        QTimer::singleShot(0, this, &ListsListModel::searchActorInEachLists);
    }
}

bool ListsListModel::checkVisibility(const QString &cid)
{
    if (visibilityType() == VisibilityTypeAll) {
        if (searchTarget().startsWith("did:")) {
            m_searchCidQue.insert(0, cid);
        }
        return true;
    }

    const AppBskyGraphDefs::ListView &current = m_listViewHash.value(cid);

    if ((visibilityType() == VisibilityTypeCuration
         && current.purpose == "app.bsky.graph.defs#curatelist")
        || (visibilityType() == VisibilityTypeModeration
            && current.purpose == "app.bsky.graph.defs#modlist")) {
        if (searchTarget().startsWith("did:")) {
            m_searchCidQue.insert(0, cid);
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
    if (m_searchCidQue.isEmpty()) {
        setRunning(false);
        return;
    }

    QString cid = m_searchCidQue.first();
    m_searchCidQue.pop_front();

    if (!m_listViewHash.contains(cid)) {
        update(indexOf(cid), ListsListModel::SearchStatusRole,
               SearchStatusType::SearchStatusTypeNotContains);
        setRunning(false);
        m_searchCidQue.clear();
        return;
    }
    update(indexOf(cid), ListsListModel::SearchStatusRole,
           SearchStatusType::SearchStatusTypeRunning);

    ListItemListModel *list = new ListItemListModel(this);
    connect(list, &ListItemListModel::finished, [=](bool success) {
        if (success) {
            QString listitem_did;
            QString listitem_uri;
            bool found = false;
            for (int i = 0; i < list->rowCount(); i++) {
                listitem_did = list->item(i, ListItemListModel::DidRole).toString();
                listitem_uri = ""; // TODO listRecordsで取得できるように変更する
                qDebug() << "DID?" << listitem_did << searchTarget();
                if (listitem_did == searchTarget()) {
                    // hit
                    update(indexOf(cid), ListsListModel::SearchStatusRole,
                           SearchStatusType::SearchStatusTypeContains);
                    update(indexOf(cid), ListsListModel::ListItemUriRole, listitem_uri);
                    found = true;
                    break;
                }
            }
            if (!found) {
                update(indexOf(cid), ListsListModel::SearchStatusRole,
                       SearchStatusType::SearchStatusTypeNotContains);
            }
        } else {
            m_searchCidQue.clear();
        }
        QTimer::singleShot(0, this, &ListsListModel::searchActorInEachLists);
        list->deleteLater();
    });
    list->setAccount(account().service, account().did, account().handle, account().email,
                     account().accessJwt, account().refreshJwt);
    list->setUri(m_listViewHash.value(cid).uri);
    if (!list->getLatest()) {
        m_searchCidQue.clear();
        setRunning(false);
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
