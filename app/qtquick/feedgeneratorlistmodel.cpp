#include "feedgeneratorlistmodel.h"

#include "atprotocol/app/bsky/actor/appbskyactorgetpreferences.h"
#include "atprotocol/app/bsky/unspecced/appbskyunspeccedgetpopularfeedgenerators.h"

#include <QPointer>

using AtProtocolInterface::AppBskyActorGetPreferences;
using AtProtocolInterface::AppBskyUnspeccedGetPopularFeedGenerators;
using namespace AtProtocolType::AppBskyFeedDefs;

FeedGeneratorListModel::FeedGeneratorListModel(QObject *parent)
    : AtpAbstractListModel { parent } { }

int FeedGeneratorListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_cidList.count();
}

QVariant FeedGeneratorListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<FeedGeneratorListModelRoles>(role));
}

QVariant FeedGeneratorListModel::item(int row, FeedGeneratorListModelRoles role) const
{
    if (row < 0 || row >= m_cidList.count())
        return QVariant();

    const GeneratorView &generator = m_generatorViewHash.value(m_cidList.at(row));

    if (role == UriRole)
        return generator.uri;
    else if (role == CidRole)
        return generator.cid;
    else if (role == CreatorHandleRole)
        return generator.creator.handle;
    else if (role == CreatorDisplayNameRole)
        return generator.creator.displayName;
    else if (role == CreatorAvatarRole)
        return generator.creator.avatar;
    else if (role == DisplayNameRole)
        return generator.displayName;
    else if (role == DescriptionRole)
        return generator.description;
    else if (role == LikeCountRole)
        return generator.likeCount;
    else if (role == AvatarRole)
        return generator.avatar;
    else if (role == SavingRole)
        return m_savedUriList.contains(generator.uri);

    return QVariant();
}

int FeedGeneratorListModel::indexOf(const QString &cid) const
{
    Q_UNUSED(cid)
    return -1;
}

QString FeedGeneratorListModel::getRecordText(const QString &cid)
{
    Q_UNUSED(cid)
    return QString();
}

void FeedGeneratorListModel::clear()
{
    if (!m_cidList.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_cidList.count() - 1);
        m_cidList.clear();
        endRemoveRows();
    }
    m_generatorViewHash.clear();
    m_savedUriList.clear();
}

void FeedGeneratorListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    clear();

    QPointer<FeedGeneratorListModel> aliving(this);

    AppBskyUnspeccedGetPopularFeedGenerators *generators =
            new AppBskyUnspeccedGetPopularFeedGenerators();
    connect(generators, &AppBskyUnspeccedGetPopularFeedGenerators::finished, [=](bool success) {
        if (aliving) {
            if (success) {
                beginInsertRows(QModelIndex(), 0, generators->generatorViewList()->count() - 1);
                for (const auto &generator : *generators->generatorViewList()) {
                    m_cidList.append(generator.cid);
                    m_generatorViewHash[generator.cid] = generator;
                }
                endInsertRows();

                getSavedGenerators();
            } else {
                setRunning(false);
            }
        }
        generators->deleteLater();
    });
    generators->setAccount(account());
    generators->getPopularFeedGenerators();
}

QHash<int, QByteArray> FeedGeneratorListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[UriRole] = "uri";
    roles[CidRole] = "cid";
    roles[CreatorHandleRole] = "creatorHandle";
    roles[CreatorDisplayNameRole] = "creatorDisplayName";
    roles[CreatorAvatarRole] = "creatorAvatar";
    roles[DisplayNameRole] = "displayName";
    roles[DescriptionRole] = "description";
    roles[LikeCountRole] = "likeCount";
    roles[AvatarRole] = "avatar";
    roles[SavingRole] = "saving";

    return roles;
}

void FeedGeneratorListModel::getSavedGenerators()
{
    QPointer<FeedGeneratorListModel> aliving(this);

    AppBskyActorGetPreferences *pref = new AppBskyActorGetPreferences();
    connect(pref, &AppBskyActorGetPreferences::finished, [=](bool success) {
        if (aliving) {
            if (success) {
                for (const auto &feed : *pref->savedFeedsPrefList()) {
                    m_savedUriList.append(feed.saved);

                    QHashIterator<QString, AtProtocolType::AppBskyFeedDefs::GeneratorView> i(
                            m_generatorViewHash);
                    while (i.hasNext()) {
                        i.next();
                        int pos = feed.saved.indexOf(i.value().uri);
                        if (pos >= 0) {
                            emit dataChanged(index(pos), index(pos),
                                             QVector<int>() << static_cast<int>(SavingRole));
                        }
                    }
                }
            }
            setRunning(false);
        }
        pref->deleteLater();
    });
    pref->setAccount(account());
    pref->getPreferences();
}
