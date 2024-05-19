#include "listslistmodel.h"
#include "recordoperator.h"
#include "tools/listitemscache.h"

#include "atprotocol/app/bsky/graph/appbskygraphgetlists.h"
#include "extension/com/atproto/repo/comatprotorepolistrecordsex.h"
#include "atprotocol/app/bsky/graph/appbskygraphmuteactorlist.h"
#include "atprotocol/app/bsky/graph/appbskygraphunmuteactorlist.h"
#include "recordoperator.h"

using AtProtocolInterface::AppBskyGraphGetLists;
using AtProtocolInterface::AppBskyGraphMuteActorList;
using AtProtocolInterface::AppBskyGraphUnmuteActorList;
using AtProtocolInterface::ComAtprotoRepoListRecordsEx;
using namespace AtProtocolType;

ListsListModel::ListsListModel(QObject *parent)
    : AtpAbstractListModel { parent },
      m_visibilityType(ListsListModel::VisibilityTypeAll),
      m_checkedCount(0)
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
    else if (role == MutedRole)
        return current.viewer.muted;
    else if (role == BlockedRole)
        return current.viewer.blocked.contains(did()) && !did().isEmpty();
    else if (role == BlockedUriRole)
        return current.viewer.blocked;
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
    } else if (role == CreatorAvatarRole) {
        if (!current.creator)
            return QString();
        else
            return current.creator->avatar;
    } else if (role == IsModerationRole) {
        return (toVisibilityType(current.purpose) == VisibilityTypeModeration);
    } else if (role == SearchStatusRole) {
        return m_searchStatusHash.value(current.cid, SearchStatusTypeUnknown);
    } else if (role == ListItemUriRole) {
        return m_listItemUriHash.value(current.cid, QString());
    } else if (role == CheckedRole) {
        return m_checkedHash.value(current.cid, false);
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
        QString new_uri = value.toString();
        if (!m_listItemUriHash.contains(current.cid) || m_listItemUriHash[current.cid] != new_uri) {
            m_listItemUriHash[current.cid] = new_uri;
            emit dataChanged(index(row), index(row));
        }
    } else if (role == MutedRole) {
        m_listViewHash[current.cid].viewer.muted = value.toBool();
        emit dataChanged(index(row), index(row));
    } else if (role == BlockedUriRole) {
        m_listViewHash[current.cid].viewer.blocked = value.toString();
        emit dataChanged(index(row), index(row));
    } else if (role == CheckedRole) {
        m_checkedHash[current.cid] = value.toBool();
        emit dataChanged(index(row), index(row));

        int count = 0;
        for (const auto &cid : m_cidList) {
            count += m_checkedHash.value(cid, false) ? 1 : 0;
        }
        setCheckedCount(count);
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
    m_listItemCursor.clear();
    m_checkedHash.clear();
    AtpAbstractListModel::clear();
}

void ListsListModel::clearListItemCache()
{
    ListItemsCache::getInstance()->clear(account().did);
}

bool ListsListModel::addRemoveFromList(const int row, const QString &did)
{
    QString list_name = item(row, ListsListModel::NameRole).toString();
    QString list_uri = item(row, ListsListModel::UriRole).toString();
    QString target_uri = list_uri;
    SearchStatusType status =
            static_cast<SearchStatusType>(item(row, ListsListModel::SearchStatusRole).toInt());
    if (status == SearchStatusTypeContains) {
        target_uri = item(row, ListsListModel::ListItemUriRole).toString();
    }
    if (running() || target_uri.isEmpty())
        return false;

    RecordOperator *ope = new RecordOperator(this);
    connect(ope, &RecordOperator::finished,
            [=](bool success, const QString &uri, const QString &cid) {
                Q_UNUSED(cid)
                if (success) {
                    if (status == SearchStatusTypeContains) {
                        // Deleted
                        update(row, ListsListModel::SearchStatusRole,
                               SearchStatusType::SearchStatusTypeNotContains);
                        update(row, ListsListModel::ListItemUriRole, QString());
                        // delete from cache
                        ListItemsCache::getInstance()->removeItem(account().did, did, list_uri);
                    } else {
                        // Added
                        update(row, ListsListModel::SearchStatusRole,
                               SearchStatusType::SearchStatusTypeContains);
                        update(row, ListsListModel::ListItemUriRole, uri);
                        // add to cache
                        ListItemsCache::getInstance()->addItem(account().did, did, list_name,
                                                               list_uri, uri);
                    }
                }
                setRunning(false);
                ope->deleteLater();
            });
    ope->setAccount(account().service, account().did, account().handle, account().email,
                    account().accessJwt, account().refreshJwt);
    if (status == SearchStatusTypeContains) {
        // delete
        // uriのレコードを消す（リストに登録しているユーザーの情報）
        update(row, ListsListModel::SearchStatusRole, SearchStatusType::SearchStatusTypeRunning);
        setRunning(ope->deleteListItem(target_uri));
    } else if (status == SearchStatusTypeNotContains) {
        // Add
        // uriのリストにdidのユーザーを追加する
        update(row, ListsListModel::SearchStatusRole, SearchStatusType::SearchStatusTypeRunning);
        setRunning(ope->listItem(target_uri, did));
    } else {
        ope->deleteLater();
    }
    return running();
}

void ListsListModel::mute(const int row)
{
    QVariant uri = item(row, ListsListModel::UriRole);
    QVariant muted = item(row, ListsListModel::MutedRole);
    if (uri.isNull())
        return;
    if (!uri.toString().startsWith("at://"))
        return;
    if (running())
        return;
    setRunning(true);

    if (muted.toBool()) {
        // -> unmute
        AppBskyGraphUnmuteActorList *list = new AppBskyGraphUnmuteActorList(this);
        connect(list, &AppBskyGraphUnmuteActorList::finished, [=](bool success) {
            if (success) {
                update(row, ListsListModel::MutedRole, false);
                setRunning(false);
            }
            list->deleteLater();
        });
        list->setAccount(account());
        list->unmuteActorList(uri.toString());
    } else {
        // -> mute
        AppBskyGraphMuteActorList *list = new AppBskyGraphMuteActorList(this);
        connect(list, &AppBskyGraphMuteActorList::finished, [=](bool success) {
            if (success) {
                update(row, ListsListModel::MutedRole, true);
                setRunning(false);
            }
            list->deleteLater();
        });
        list->setAccount(account());
        list->muteActorList(uri.toString());
    }
}

void ListsListModel::block(const int row)
{
    QVariant uri = item(row, ListsListModel::UriRole);
    QVariant blocked = item(row, ListsListModel::BlockedRole);
    if (uri.isNull())
        return;
    if (!uri.toString().startsWith("at://"))
        return;
    if (running())
        return;
    setRunning(true);

    RecordOperator *ope = new RecordOperator(this);
    connect(ope, &RecordOperator::finished,
            [=](bool success, const QString &uri, const QString &cid) {
                qDebug() << success << uri;
                if (success) {
                    update(row, ListsListModel::BlockedUriRole, uri);
                    setRunning(false);
                }
                ope->deleteLater();
            });
    ope->setAccount(account().service, account().did, account().handle, account().email,
                    account().accessJwt, account().refreshJwt);
    if (blocked.toBool()) {
        // -> unblock
        ope->deleteBlockList(item(row, ListsListModel::BlockedUriRole).toString());
    } else {
        // -> block
        ope->blockList(uri.toString());
    }
}

ListsListModel::VisibilityType ListsListModel::toVisibilityType(const QString &purpose) const
{
    if (purpose == "app.bsky.graph.defs#curatelist") {
        return VisibilityTypeCuration;
    } else if (purpose == "app.bsky.graph.defs#modlist") {
        return VisibilityTypeModeration;
    } else {
        return VisibilityTypeUnknown;
    }
}

bool ListsListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    m_searchCidQue.clear();
    m_listItemCursor.clear();

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
    lists->getLists(actor(), 0, QString());

    return true;
}

bool ListsListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    m_searchCidQue.clear();
    AppBskyGraphGetLists *lists = new AppBskyGraphGetLists(this);
    connect(lists, &AppBskyGraphGetLists::finished, [=](bool success) {
        if (success) {
            m_cursor = lists->cursor(); // 続きの読み込みの時は必ず上書き
            if (lists->listsListViewList().isEmpty())
                m_cursor.clear(); // すべて読み切って空になったときはカーソルこないので空になるはずだけど念のため
            copyFrom(lists);
            QTimer::singleShot(10, this, &ListsListModel::displayQueuedPostsNext);
        } else {
            m_cursor.clear();
            emit errorOccured(lists->errorCode(), lists->errorMessage());
            setRunning(false);
        }
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
    roles[MutedRole] = "muted";
    roles[BlockedRole] = "blocked";
    roles[BlockedUriRole] = "blockedUri";
    roles[CreatorHandleRole] = "creatorHandle";
    roles[CreatorDisplayNameRole] = "creatorDisplayName";
    roles[CreatorAvatarRole] = "creatoravatar";
    roles[IsModerationRole] = "isModeration";
    roles[SearchStatusRole] = "searchStatus";
    roles[ListItemUriRole] = "listItemUri";
    roles[CheckedRole] = "checked";

    return roles;
}

bool ListsListModel::aggregateQueuedPosts(const QString &cid, const bool next)
{
    return true;
}

bool ListsListModel::aggregated(const QString &cid) const
{
    return false;
}

void ListsListModel::finishedDisplayingQueuedPosts()
{
    if (searchTarget().isEmpty()) {
        setRunning(false);
    } else {
        if (!m_cursor.isEmpty()) {
            setRunning(false);
            QTimer::singleShot(0, this, &ListsListModel::getNext);
        } else if (ListItemsCache::getInstance()->has(account().did)) {
            searchActorInEachListsFromCache();
        } else {
            m_listItemCursor = QStringLiteral("__start__");
            setListItemStatus(SearchStatusTypeRunning);
            QTimer::singleShot(0, this, &ListsListModel::searchActorInEachLists);
        }
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

    if (visibilityType() == toVisibilityType(current.purpose)) {
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
    for (const auto &list : lists->listsListViewList()) {
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
    if (m_listItemCursor.isEmpty()) {
        setListItemStatus(SearchStatusTypeNotContains);
        setRunning(false);
        return;
    }
    QString cursor = m_listItemCursor;
    if (m_listItemCursor == "__start__") {
        cursor.clear();
        m_listItemCursor.clear();
    }
    AtProtocolInterface::ComAtprotoRepoListRecordsEx *list =
            new AtProtocolInterface::ComAtprotoRepoListRecordsEx(this);
    connect(list, &AtProtocolInterface::ComAtprotoRepoListRecordsEx::finished, [=](bool success) {
        if (success) {
            m_listItemCursor = list->cursor();
            if (list->recordsRecordList().isEmpty())
                m_listItemCursor.clear();

            for (const auto &item : list->recordsRecordList()) {
                AppBskyGraphListitem::Main record =
                        AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                                AppBskyGraphListitem::Main>(item.value);
                QString list_cid = getListCidByUri(record.list);
                if (!list_cid.isEmpty()) {
                    const AppBskyGraphDefs::ListView &current = m_listViewHash.value(list_cid);
                    if (record.subject == searchTarget()) {
                        update(indexOf(list_cid), ListsListModel::ListItemUriRole, item.uri);
                        update(indexOf(list_cid), ListsListModel::SearchStatusRole,
                               SearchStatusType::SearchStatusTypeContains);
                        // Listを横断してListItemを探索するのでbreakはできない
                    }
                    // キャッシュに登録
                    ListItemsCache::getInstance()->addItem(account().did, record.subject,
                                                           current.name, current.uri, item.uri);
                }
            }
            QTimer::singleShot(0, this, &ListsListModel::searchActorInEachLists);
        } else {
            setRunning(false);
        }
        list->deleteLater();
    });
    list->setAccount(account());
    list->listListItems(account().did, cursor);
}

void ListsListModel::searchActorInEachListsFromCache()
{
    setListItemStatus(SearchStatusTypeRunning);

    QStringList belonging_uris =
            ListItemsCache::getInstance()->getListUris(account().did, searchTarget());
    if (belonging_uris.isEmpty()) {
        setListItemStatus(SearchStatusTypeNotContains);
        setRunning(false);
        return;
    }

    for (const QString &cid : m_cidList) {
        const AppBskyGraphDefs::ListView &current = m_listViewHash.value(cid);
        if (belonging_uris.contains(current.uri)) {
            ListInfo info = ListItemsCache::getInstance()->getListInfo(account().did,
                                                                       searchTarget(), current.uri);
            if (!info.item_uri.isEmpty()) {
                update(indexOf(cid), ListsListModel::ListItemUriRole, info.item_uri);
                update(indexOf(cid), ListsListModel::SearchStatusRole,
                       SearchStatusType::SearchStatusTypeContains);
            } else {
                update(indexOf(cid), ListsListModel::SearchStatusRole,
                       SearchStatusType::SearchStatusTypeNotContains);
            }
        }
    }

    setListItemStatus(SearchStatusTypeNotContains);
    setRunning(false);
}

QString ListsListModel::getListCidByUri(const QString &uri) const
{
    for (const auto &cid : m_cidList) {
        if (m_listViewHash.value(cid).uri == uri) {
            return cid;
        }
    }
    return QString();
}

void ListsListModel::setListItemStatus(const SearchStatusType status)
{
    for (int i = 0; i < m_cidList.count(); i++) {
        SearchStatusType now_status =
                static_cast<SearchStatusType>(item(i, SearchStatusRole).toInt());
        if (status == SearchStatusTypeUnknown || status == SearchStatusTypeRunning) {
            update(i, ListsListModel::SearchStatusRole, status);
        } else if (status == SearchStatusTypeNotContains) {
            if (now_status == SearchStatusTypeRunning) {
                update(i, ListsListModel::SearchStatusRole, status);
            }
        }
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

int ListsListModel::checkedCount() const
{
    return m_checkedCount;
}

void ListsListModel::setCheckedCount(int newCheckedCount)
{
    if (m_checkedCount == newCheckedCount)
        return;
    m_checkedCount = newCheckedCount;
    emit checkedCountChanged();
}
