#include "listslistmodel.h"
// #include "listitemlistmodel.h"
#include "recordoperator.h"

#include "atprotocol/app/bsky/graph/appbskygraphgetlists.h"
#include "atprotocol/com/atproto/repo/comatprotorepolistrecords.h"

using AtProtocolInterface::AppBskyGraphGetLists;
using AtProtocolInterface::ComAtprotoRepoListRecords;
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

bool ListsListModel::addRemoveFromList(const int row, const QString &did)
{
    QString target_uri = item(row, ListsListModel::UriRole).toString();
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
                        update(row, ListsListModel::SearchStatusRole,
                               SearchStatusType::SearchStatusTypeNotContains);
                        update(row, ListsListModel::ListItemUriRole, QString());
                    } else {
                        update(row, ListsListModel::SearchStatusRole,
                               SearchStatusType::SearchStatusTypeContains);
                        update(row, ListsListModel::ListItemUriRole, uri);
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
        setRunning(ope->deleteListItem(target_uri));
    } else if (status == SearchStatusTypeNotContains) {
        // Add
        // uriのリストにdidのユーザーを追加する
        setRunning(ope->listItem(target_uri, did));
    } else {
        ope->deleteLater();
    }
    return running();
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
            if (lists->listViewList()->isEmpty())
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
    roles[SubscribedRole] = "subscribed";
    roles[CreatorHandleRole] = "creatorHandle";
    roles[CreatorDisplayNameRole] = "creatorDisplayName";
    roles[CreatoravatarRole] = "creatoravatar";
    roles[SearchStatusRole] = "searchStatus";
    roles[ListItemUriRole] = "listItemUri";
    roles[CheckedRole] = "checked";

    return roles;
}

void ListsListModel::finishedDisplayingQueuedPosts()
{
    if (searchTarget().isEmpty()) {
        setRunning(false);
    } else {
        if (!m_cursor.isEmpty()) {
            setRunning(false);
            QTimer::singleShot(0, this, &ListsListModel::getNext);
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
    AtProtocolInterface::ComAtprotoRepoListRecords *list =
            new AtProtocolInterface::ComAtprotoRepoListRecords(this);
    connect(list, &AtProtocolInterface::ComAtprotoRepoListRecords::finished, [=](bool success) {
        if (success) {
            m_listItemCursor = list->cursor();
            if (list->recordList()->isEmpty())
                m_listItemCursor.clear();

            for (const auto &item : *list->recordList()) {
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
