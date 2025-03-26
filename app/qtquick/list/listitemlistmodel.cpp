#include "listitemlistmodel.h"
#include "atprotocol/app/bsky/graph/appbskygraphmuteactorlist.h"
#include "atprotocol/app/bsky/graph/appbskygraphunmuteactorlist.h"
#include "operation/recordoperator.h"

using AtProtocolInterface::AppBskyGraphGetList;
using AtProtocolInterface::AppBskyGraphMuteActorList;
using AtProtocolInterface::AppBskyGraphUnmuteActorList;
using namespace AtProtocolType;

ListItemListModel::ListItemListModel(QObject *parent)
    : AtpAbstractListModel { parent },
      m_subscribed(false),
      m_isModeration(false),
      m_muted(false),
      m_blocked(false)
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

QString ListItemListModel::getOfficialUrl() const
{
    return atUriToOfficialUrl(uri(), QStringLiteral("lists"));
}

void ListItemListModel::clear()
{
    m_listItemViewHash.clear();
    AtpAbstractListModel::clear();
}

void ListItemListModel::mute()
{
    if (!uri().startsWith("at://"))
        return;
    if (running())
        return;
    setRunning(true);

    if (muted()) {
        // -> unmute
        AppBskyGraphUnmuteActorList *list = new AppBskyGraphUnmuteActorList(this);
        connect(list, &AppBskyGraphUnmuteActorList::finished, [=](bool success) {
            if (success) {
                setMuted(false);
                setRunning(false);
            }
            list->deleteLater();
        });
        list->setAccount(account());
        list->unmuteActorList(uri());
    } else {
        // -> mute
        AppBskyGraphMuteActorList *list = new AppBskyGraphMuteActorList(this);
        connect(list, &AppBskyGraphMuteActorList::finished, [=](bool success) {
            if (success) {
                setMuted(true);
                setRunning(false);
            }
            list->deleteLater();
        });
        list->setAccount(account());
        list->muteActorList(uri());
    }
}

void ListItemListModel::block()
{
    if (!uri().startsWith("at://"))
        return;
    if (running())
        return;
    setRunning(true);

    RecordOperator *ope = new RecordOperator(this);
    connect(ope, &RecordOperator::finished, [=](bool success) {
        if (success) {
            setBlocked(!blocked());
            setRunning(false);
        }
        ope->deleteLater();
    });
    ope->setAccount(account().uuid);
    if (blocked()) {
        // -> unblock
        ope->deleteBlockList(blockedUri());
    } else {
        // -> block
        ope->blockList(uri());
    }
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
            QTimer::singleShot(10, this, &ListItemListModel::displayQueuedPosts);
        } else {
            emit errorOccured(list->errorCode(), list->errorMessage());
            setRunning(false);
            emit finished(false);
        }
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
            QTimer::singleShot(10, this, &ListItemListModel::displayQueuedPostsNext);
        } else {
            emit errorOccured(list->errorCode(), list->errorMessage());
            setRunning(false);
            emit finished(false);
        }
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

bool ListItemListModel::aggregateQueuedPosts(const QString &cid, const bool next)
{
    Q_UNUSED(cid)
    Q_UNUSED(next)
    return true;
}

bool ListItemListModel::aggregated(const QString &cid) const
{
    Q_UNUSED(cid)
    return false;
}

void ListItemListModel::finishedDisplayingQueuedPosts()
{
    setRunning(false);
    emit finished(true);
}

bool ListItemListModel::checkVisibility(const QString &cid)
{
    Q_UNUSED(cid)
    return true;
}

void ListItemListModel::copyFrom(AtProtocolInterface::AppBskyGraphGetList *list)
{
    setUri(list->list().uri);
    setCid(list->list().cid);
    setName(list->list().name);
    setAvatar(list->list().avatar);
    setDescription(list->list().description);
    setMuted(list->list().viewer.muted);
    setBlocked(list->list().viewer.blocked.contains(did()) && !did().isEmpty());
    setBlockedUri(list->list().viewer.blocked);
    setCreatorDid(list->list().creator->did);
    setCreatorHandle(list->list().creator->handle);
    setCreatorDisplayName(list->list().creator->displayName);
    setIsModeration((list->list().purpose == "app.bsky.graph.defs#modlist"));

    for (const auto &item : list->itemsList()) {
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

bool ListItemListModel::isModeration() const
{
    return m_isModeration;
}

void ListItemListModel::setIsModeration(bool newIsModeration)
{
    if (m_isModeration == newIsModeration)
        return;
    m_isModeration = newIsModeration;
    emit isModerationChanged();
}

bool ListItemListModel::muted() const
{
    return m_muted;
}

void ListItemListModel::setMuted(bool newMuted)
{
    if (m_muted == newMuted)
        return;
    m_muted = newMuted;
    emit mutedChanged();
}

bool ListItemListModel::blocked() const
{
    return m_blocked;
}

void ListItemListModel::setBlocked(bool newBlocked)
{
    if (m_blocked == newBlocked)
        return;
    m_blocked = newBlocked;
    emit blockedChanged();
}

QString ListItemListModel::blockedUri() const
{
    return m_blockedUri;
}

void ListItemListModel::setBlockedUri(const QString &newBlockedUri)
{
    if (m_blockedUri == newBlockedUri)
        return;
    m_blockedUri = newBlockedUri;
    emit blockedUriChanged();
}
