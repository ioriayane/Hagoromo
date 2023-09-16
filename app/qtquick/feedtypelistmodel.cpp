#include "feedtypelistmodel.h"

#include "atprotocol/lexicons.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetpreferences.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerators.h"

using AtProtocolInterface::AppBskyActorGetPreferences;
using AtProtocolInterface::AppBskyFeedGetFeedGenerators;
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

    if (role == FeedTypeListModelRoles::FeedTypeRole)
        return static_cast<int>(m_feedTypeItemList.at(row).type);
    else if (role == FeedTypeListModelRoles::DisplayNameRole)
        return m_feedTypeItemList.at(row).generator.displayName;
    else if (role == FeedTypeListModelRoles::AvatarRole)
        return m_feedTypeItemList.at(row).generator.avatar;
    else if (role == FeedTypeListModelRoles::UriRole)
        return m_feedTypeItemList.at(row).generator.uri;
    else if (role == FeedTypeListModelRoles::CreatorDisplayNameRole)
        return m_feedTypeItemList.at(row).generator.creator.displayName;

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
    if (!m_feedTypeItemList.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        m_feedTypeItemList.clear();
        endRemoveRows();
    }
    beginInsertRows(QModelIndex(), 0, 1);
    {
        FeedTypeItem item;
        item.type = FeedComponentType::Timeline;
        item.generator.displayName = tr("Following");
        m_feedTypeItemList.append(item);
    }
    {
        FeedTypeItem item;
        item.type = FeedComponentType::Notification;
        item.generator.displayName = tr("Notification");
        m_feedTypeItemList.append(item);
    }
    endInsertRows();
}

void FeedTypeListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    clear();

    AppBskyActorGetPreferences *pref = new AppBskyActorGetPreferences(this);
    connect(pref, &AppBskyActorGetPreferences::finished, [=](bool success) {
        if (success) {
            for (const auto &feed : *pref->savedFeedsPrefList()) {
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
}

void FeedTypeListModel::getNext()
{
    //
}

QHash<int, QByteArray> FeedTypeListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

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
        setRunning(false);
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
                for (const auto &generator : *generators->generatorViewList()) {
                    if (uri == generator.uri) {
                        FeedTypeItem item;
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
