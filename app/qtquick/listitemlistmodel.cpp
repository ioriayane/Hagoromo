#include "listitemlistmodel.h"

using AtProtocolInterface::AppBskyGraphGetList;
using namespace AtProtocolType;

ListItemListModel::ListItemListModel(QObject *parent)
    : AtpAbstractListModel { parent }, m_subscribed(false)
{
}

int ListItemListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_cidList.count();
}

QVariant ListItemListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<ListItemListModelRoles>(role));
}

QVariant ListItemListModel::item(int row, ListItemListModelRoles role) const
{
    if (row < 0 || row >= m_cidList.count())
        return QVariant();

    const AppBskyGraphDefs::ListItemView &current = m_listItemViewHash.value(m_cidList.at(row));

    if (current.subject.isNull())
        return QVariant();
    else if (role == DidRole)
        return current.subject->did;
    else if (role == HandleRole)
        return current.subject->handle;
    else if (role == DisplayNameRole)
        return current.subject->displayName;
    else if (role == DescriptionRole)
        return m_systemTool.markupText(current.subject->description);
    else if (role == AvatarRole)
        return current.subject->avatar;
    else if (role == IndexedAtRole)
        return current.subject->indexedAt;

    return QVariant();
}

int ListItemListModel::indexOf(const QString &cid) const
{
    return m_cidList.indexOf(cid);
}

QString ListItemListModel::getRecordText(const QString &cid)
{
    Q_UNUSED(cid)
    return QString();
}

void ListItemListModel::clear()
{
    m_listItemViewHash.clear();
    AtpAbstractListModel::clear();
}

bool ListItemListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    AppBskyGraphGetList *list = new AppBskyGraphGetList(this);
    connect(list, &AppBskyGraphGetList::finished, [=](bool success) {
        if (success) {
            if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                m_cursor = list->cursor();
            }
            copyFrom(list);
        }
        QTimer::singleShot(10, this, &ListItemListModel::displayQueuedPosts);
        list->deleteLater();
    });
    list->setAccount(account());
    list->getList(uri(), 100, QString());

    return true;
}

bool ListItemListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    AppBskyGraphGetList *list = new AppBskyGraphGetList(this);
    connect(list, &AppBskyGraphGetList::finished, [=](bool success) {
        if (success) {
            m_cursor = list->cursor();
            copyFrom(list);
        }
        QTimer::singleShot(10, this, &ListItemListModel::displayQueuedPostsNext);
        list->deleteLater();
    });
    list->setAccount(account());
    list->getList(uri(), 100, m_cursor);

    return true;
}

QHash<int, QByteArray> ListItemListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[DidRole] = "did";
    roles[HandleRole] = "handle";
    roles[DisplayNameRole] = "displayName";
    roles[DescriptionRole] = "description";
    roles[AvatarRole] = "avatar";
    roles[IndexedAtRole] = "indexedAt";

    return roles;
}

void ListItemListModel::finishedDisplayingQueuedPosts()
{
    setRunning(false);
}

bool ListItemListModel::checkVisibility(const QString &cid)
{
    return true;
}

void ListItemListModel::copyFrom(AtProtocolInterface::AppBskyGraphGetList *list)
{
    setUri(list->listView()->uri);
    setCid(list->listView()->cid);
    setName(list->listView()->name);
    setAvatar(list->listView()->avatar);
    setDescription(list->listView()->description);
    setSubscribed(list->listView()->viewer.muted);
    setCreatorDid(list->listView()->creator->did);
    setCreatorHandle(list->listView()->creator->handle);
    setCreatorDisplayName(list->listView()->creator->displayName);

    for (const auto &item : *list->listItemViewList()) {
        if (!item.subject)
            continue;
        m_listItemViewHash[item.subject->did] = item;

        PostCueItem post;
        post.cid = item.subject->did;
        post.indexed_at = item.subject->indexedAt;
        post.reference_time = QDateTime::currentDateTimeUtc();
        m_cuePost.insert(0, post);
    }
}

QString ListItemListModel::uri() const
{
    return m_uri;
}

void ListItemListModel::setUri(const QString &newUri)
{
    if (m_uri == newUri)
        return;
    m_uri = newUri;
    emit uriChanged();

    if (m_uri.startsWith("at://")) {
        QStringList items = m_uri.split("/");
        if (!items.isEmpty()) {
            setRkey(items.last());
        }
    }
}

QString ListItemListModel::cid() const
{
    return m_cid;
}

void ListItemListModel::setCid(const QString &newCid)
{
    if (m_cid == newCid)
        return;
    m_cid = newCid;
    emit cidChanged();
}

QString ListItemListModel::rkey() const
{
    return m_rkey;
}

void ListItemListModel::setRkey(const QString &newRkey)
{
    if (m_rkey == newRkey)
        return;
    m_rkey = newRkey;
    emit rkeyChanged();
}

QString ListItemListModel::name() const
{
    return m_name;
}

void ListItemListModel::setName(const QString &newName)
{
    if (m_name == newName)
        return;
    m_name = newName;
    emit nameChanged();
}

QString ListItemListModel::avatar() const
{
    return m_avatar;
}

void ListItemListModel::setAvatar(const QString &newAvatar)
{
    if (m_avatar == newAvatar)
        return;
    m_avatar = newAvatar;
    emit avatarChanged();
}

QString ListItemListModel::description() const
{
    return m_formattedDescription;
}

void ListItemListModel::setDescription(const QString &newDescription)
{
    if (m_description == newDescription)
        return;
    m_description = newDescription;
    m_formattedDescription = m_systemTool.markupText(m_description);
    emit descriptionChanged();
}

bool ListItemListModel::subscribed() const
{
    return m_subscribed;
}

void ListItemListModel::setSubscribed(bool newSubscribed)
{
    if (m_subscribed == newSubscribed)
        return;
    m_subscribed = newSubscribed;
    emit subscribedChanged();
}

QString ListItemListModel::creatorDid() const
{
    return m_creatorDid;
}

void ListItemListModel::setCreatorDid(const QString &newCreatorDid)
{
    if (m_creatorDid == newCreatorDid)
        return;
    m_creatorDid = newCreatorDid;
    emit creatorDidChanged();
}

QString ListItemListModel::creatorHandle() const
{
    return m_creatorHandle;
}

void ListItemListModel::setCreatorHandle(const QString &newCreatorHandle)
{
    if (m_creatorHandle == newCreatorHandle)
        return;
    m_creatorHandle = newCreatorHandle;
    emit creatorHandleChanged();
}

QString ListItemListModel::creatorDisplayName() const
{
    return m_creatorDisplayName;
}

void ListItemListModel::setCreatorDisplayName(const QString &newCreatorDisplayName)
{
    if (m_creatorDisplayName == newCreatorDisplayName)
        return;
    m_creatorDisplayName = newCreatorDisplayName;
    emit creatorDisplayNameChanged();
}
