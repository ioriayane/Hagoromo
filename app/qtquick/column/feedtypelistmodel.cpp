#include "feedtypelistmodel.h"

#include "atprotocol/lexicons.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetpreferences.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerators.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetlists.h"

using AtProtocolInterface::AppBskyActorGetPreferences;
using AtProtocolInterface::AppBskyFeedGetFeedGenerators;
using AtProtocolInterface::AppBskyGraphGetLists;
using namespace AtProtocolType::AppBskyActorDefs;

FeedTypeListModel::FeedTypeListModel(QObject *parent) : AtpAbstractListModel { parent }
{
    clear();
}

int FeedTypeListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_feedTypeItemList.count();
}

QVariant FeedTypeListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<FeedTypeListModelRoles>(role));
}

QVariant FeedTypeListModel::item(int row, FeedTypeListModelRoles role) const
{
    if (row < 0 || row >= m_feedTypeItemList.count())
        return QVariant();

    if (role == GroupRole)
        return m_feedTypeItemList.at(row).group;
    else if (role == FeedTypeRole)
        return static_cast<int>(m_feedTypeItemList.at(row).type);

    else if (m_feedTypeItemList.at(row).type == FeedComponentType::ListFeed) {
        if (role == DisplayNameRole)
            return m_feedTypeItemList.at(row).list.name;
        else if (role == AvatarRole)
            return m_feedTypeItemList.at(row).list.avatar;
        else if (role == UriRole)
            return m_feedTypeItemList.at(row).list.uri;
        else if (role == CreatorDisplayNameRole) {
            if (m_feedTypeItemList.at(row).list.creator) {
                return m_feedTypeItemList.at(row).list.creator->displayName;
            } else {
                return QString();
            }
        }
    } else {
        if (role == DisplayNameRole)
            return m_feedTypeItemList.at(row).generator.displayName;
        else if (role == AvatarRole)
            return m_feedTypeItemList.at(row).generator.avatar;
        else if (role == UriRole)
            return m_feedTypeItemList.at(row).generator.uri;
        else if (role == CreatorDisplayNameRole)
            return m_feedTypeItemList.at(row).generator.creator.displayName;
    }
    return QVariant();
}

int FeedTypeListModel::indexOf(const QString &cid) const
{
    Q_UNUSED(cid)
    return -1;
}

QString FeedTypeListModel::getRecordText(const QString &cid)
{
    Q_UNUSED(cid)
    return QString();
}

void FeedTypeListModel::clear()
{
    m_cursor.clear();
    if (!m_feedTypeItemList.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        m_feedTypeItemList.clear();
        endRemoveRows();
    }

    // デフォルトで追加した状態にするアイテムを増減させるときは
    // AddColumnDialog.qmlのchangeColumnTypeView()を
    // 修正しないとカスタムフィードなどが読み込まれない
    beginInsertRows(QModelIndex(), 0, 7);
    {
        FeedTypeItem item;
        item.group = tr("Default Feeds");
        item.type = FeedComponentType::Timeline;
        item.generator.displayName = tr("Following");
        m_feedTypeItemList.append(item);
    }
    {
        FeedTypeItem item;
        item.group = tr("Default Feeds");
        item.type = FeedComponentType::Notification;
        item.generator.displayName = tr("Notification");
        m_feedTypeItemList.append(item);
    }
    {
        FeedTypeItem item;
        item.group = tr("Realtime Feeds");
        item.type = FeedComponentType::RealtimeFeed;
        item.generator.displayName = tr("Following");
        item.generator.uri = "{\"or\": [{\"following\": {}},{\"me\": {}}]}";
        m_feedTypeItemList.append(item);
    }
    {
        FeedTypeItem item;
        item.group = tr("Realtime Feeds");
        item.type = FeedComponentType::RealtimeFeed;
        item.generator.displayName = tr("Followers");
        item.generator.uri = "{\"followers\": {}}";
        m_feedTypeItemList.append(item);
    }
    {
        FeedTypeItem item;
        item.group = tr("Realtime Feeds");
        item.type = FeedComponentType::RealtimeFeed;
        item.generator.displayName = tr("Mutual following");
        item.generator.uri =
                "{\"or\": [{\"and\": [{\"following\": {}},{\"followers\": {}}]},{\"me\": {}}]}";
        m_feedTypeItemList.append(item);
    }
    {
        FeedTypeItem item;
        item.group = tr("Realtime Feeds");
        item.type = FeedComponentType::RealtimeFeed;
        item.generator.displayName = tr("Following and Followers");
        item.generator.uri = "{\"or\": [{\"following\": {}},{\"followers\": {}},{\"me\": {}}]}";
        m_feedTypeItemList.append(item);
    }
    {
        FeedTypeItem item;
        item.group = tr("Realtime Feeds");
        item.type = FeedComponentType::EditRealtimeFeed;
        item.generator.displayName = tr("Create Realtime Feed");
        item.generator.uri = "";
        m_feedTypeItemList.append(item);
    }
    {
        FeedTypeItem item;
        item.group = tr("Chat");
        item.type = FeedComponentType::ChatList;
        item.generator.displayName = tr("Chat list");
        m_feedTypeItemList.append(item);
    }
    endInsertRows();
}

bool FeedTypeListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    clear();

    AppBskyActorGetPreferences *pref = new AppBskyActorGetPreferences(this);
    connect(pref, &AppBskyActorGetPreferences::finished, [=](bool success) {
        if (success) {
            for (const auto &feed : pref->preferences().savedFeedsPref) {
                m_cueUri = feed.saved;
            }
        } else {
            emit errorOccured(pref->errorCode(), pref->errorMessage());
        }
        QTimer::singleShot(10, this, &FeedTypeListModel::getFeedDetails);
        pref->deleteLater();
    });
    pref->setAccount(account());
    pref->getPreferences();

    return true;
}

bool FeedTypeListModel::getNext()
{
    return true;
}

QHash<int, QByteArray> FeedTypeListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[GroupRole] = "group";
    roles[FeedTypeRole] = "feedType";
    roles[DisplayNameRole] = "displayName";
    roles[AvatarRole] = "avatar";
    roles[UriRole] = "uri";
    roles[CreatorDisplayNameRole] = "creatorDisplayName";

    return roles;
}

bool FeedTypeListModel::checkVisibility(const QString &cid)
{
    Q_UNUSED(cid)
    return true;
}

void FeedTypeListModel::getFeedDetails()
{
    if (m_cueUri.isEmpty()) {
        FeedTypeItem item;
        item.group = tr("My Feeds");
        item.type = FeedComponentType::DiscoverFeeds;
        item.generator.displayName = tr("Discover Feeds");
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_feedTypeItemList.append(item);
        endInsertRows();

        QTimer::singleShot(10, this, &FeedTypeListModel::getLists);
        return;
    }

    // 最大25個までいっきに取得できる
    QStringList uris;
    for (int i = 0; i < 25; i++) {
        if (m_cueUri.isEmpty())
            break;
        uris.append(m_cueUri.first());
        m_cueUri.removeFirst();
    }

    AppBskyFeedGetFeedGenerators *generators = new AppBskyFeedGetFeedGenerators(this);
    connect(generators, &AppBskyFeedGetFeedGenerators::finished, [=](bool success) {
        if (success) {
            // apiで渡した順番と逆順で結果が来るので元の順番で追加する
            // 結果の仕様がいつ変わるか分からないのでAPIに投げるuriの順番で制御しない
            for (const auto &uri : qAsConst(uris)) {
                for (const auto &generator : generators->feedsList()) {
                    if (uri == generator.uri) {
                        FeedTypeItem item;
                        item.group = tr("My Feeds");
                        item.type = FeedComponentType::CustomFeed;
                        item.generator = generator;
                        beginInsertRows(QModelIndex(), rowCount(), rowCount());
                        m_feedTypeItemList.append(item);
                        endInsertRows();
                        break;
                    }
                }
            }
        } else {
            emit errorOccured(generators->errorCode(), generators->errorMessage());
        }
        QTimer::singleShot(10, this, &FeedTypeListModel::getFeedDetails);
        generators->deleteLater();
    });
    generators->setAccount(account());
    generators->getFeedGenerators(uris);
}

void FeedTypeListModel::getLists()
{
    AppBskyGraphGetLists *lists = new AppBskyGraphGetLists(this);
    connect(lists, &AppBskyGraphGetLists::finished, [=](bool success) {
        if (success) {
            if (m_cursor.isEmpty()) {
                m_cursor = lists->cursor();
            }
            for (const auto &list : lists->listsList()) {
                if (list.purpose == "app.bsky.graph.defs#modlist") {
                    continue;
                }
                FeedTypeItem item;
                item.group = tr("My Lists");
                item.type = FeedComponentType::ListFeed;
                item.list = list;
                beginInsertRows(QModelIndex(), rowCount(), rowCount());
                m_feedTypeItemList.append(item);
                endInsertRows();
            }
        } else {
            emit errorOccured(lists->errorCode(), lists->errorMessage());
        }

        setRunning(false);
        lists->deleteLater();
    });
    lists->setAccount(account());
    lists->getLists(account().did, 0, QString());
}
