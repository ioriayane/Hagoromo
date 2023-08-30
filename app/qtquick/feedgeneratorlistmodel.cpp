#include "feedgeneratorlistmodel.h"

#include "atprotocol/app/bsky/actor/appbskyactorgetpreferences.h"
#include "atprotocol/app/bsky/unspecced/appbskyunspeccedgetpopularfeedgenerators.h"
#include "atprotocol/app/bsky/actor/appbskyactorputpreferences.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

using AtProtocolInterface::AppBskyActorGetPreferences;
using AtProtocolInterface::AppBskyActorPutPreferences;
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

    AppBskyUnspeccedGetPopularFeedGenerators *generators =
            new AppBskyUnspeccedGetPopularFeedGenerators(this);
    connect(generators, &AppBskyUnspeccedGetPopularFeedGenerators::finished, [=](bool success) {
        if (success && !generators->generatorViewList()->isEmpty()) {
            beginInsertRows(QModelIndex(), 0, generators->generatorViewList()->count() - 1);
            for (const auto &generator : *generators->generatorViewList()) {
                m_cidList.append(generator.cid);
                m_generatorViewHash[generator.cid] = generator;
            }
            endInsertRows();

            m_cursor = generators->cursor();
            getSavedGenerators();
        } else {
            emit errorOccured(generators->errorMessage());
            setRunning(false);
        }
        generators->deleteLater();
    });
    generators->setAccount(account());
    generators->getPopularFeedGenerators(50, QString(), query());
}

void FeedGeneratorListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return;
    setRunning(true);

    AppBskyUnspeccedGetPopularFeedGenerators *generators =
            new AppBskyUnspeccedGetPopularFeedGenerators(this);
    connect(generators, &AppBskyUnspeccedGetPopularFeedGenerators::finished, [=](bool success) {
        if (success && !generators->generatorViewList()->isEmpty()) {
            beginInsertRows(QModelIndex(), m_cidList.count(),
                            m_cidList.count() + generators->generatorViewList()->count() - 1);
            for (const auto &generator : *generators->generatorViewList()) {
                m_cidList.append(generator.cid);
                m_generatorViewHash[generator.cid] = generator;
            }
            endInsertRows();

            m_cursor = generators->cursor();
            // getSavedGenerators();
        } else {
            m_cursor.clear();
            emit errorOccured(generators->errorMessage());
        }
        setRunning(false);
        generators->deleteLater();
    });
    generators->setAccount(account());
    generators->getPopularFeedGenerators(50, m_cursor, query());
}

void FeedGeneratorListModel::saveGenerator(const QString &uri)
{
    if (running())
        return;
    setRunning(true);

    AppBskyActorGetPreferences *pref = new AppBskyActorGetPreferences(this);
    connect(pref, &AppBskyActorGetPreferences::finished, [=](bool success) {
        if (success) {
            putPreferences(appendGeneratorToPreference(pref->replyJson(), uri));
        } else {
            emit errorOccured(pref->errorMessage());
            setRunning(false);
        }
        pref->deleteLater();
    });
    pref->setAccount(account());
    pref->getPreferences();
}

void FeedGeneratorListModel::removeGenerator(const QString &uri)
{
    if (running())
        return;
    setRunning(true);

    AppBskyActorGetPreferences *pref = new AppBskyActorGetPreferences(this);
    connect(pref, &AppBskyActorGetPreferences::finished, [=](bool success) {
        if (success) {
            putPreferences(removeGeneratorToPreference(pref->replyJson(), uri));
        } else {
            emit errorOccured(pref->errorMessage());
            setRunning(false);
        }
        pref->deleteLater();
    });
    pref->setAccount(account());
    pref->getPreferences();
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

bool FeedGeneratorListModel::checkVisibility(const QString &cid)
{
    Q_UNUSED(cid)
    return true;
}

void FeedGeneratorListModel::getSavedGenerators()
{
    m_savedUriList.clear();

    AppBskyActorGetPreferences *pref = new AppBskyActorGetPreferences(this);
    connect(pref, &AppBskyActorGetPreferences::finished, [=](bool success) {
        if (success) {
            for (const auto &feed : *pref->savedFeedsPrefList()) {
                m_savedUriList.append(feed.saved);

                //                    QHashIterator<QString,
                //                    AtProtocolType::AppBskyFeedDefs::GeneratorView> i(
                //                            m_generatorViewHash);
                //                    while (i.hasNext()) {
                //                        i.next();
                //                        int pos = feed.saved.indexOf(i.value().uri);
                //                        if (pos >= 0) {
                //                            emit dataChanged(index(pos), index(pos),
                //                                             QVector<int>() <<
                //                                             static_cast<int>(SavingRole));
                //                        }
                //                    }
            }
            for (int i = 0; i < m_cidList.count(); i++) {
                emit dataChanged(index(i), index(i),
                                 QVector<int>() << static_cast<int>(SavingRole));
            }
            //                QHashIterator<QString,
            //                AtProtocolType::AppBskyFeedDefs::GeneratorView> i(
            //                        m_generatorViewHash);
            //                while (i.hasNext()) {
            //                    i.next();
            //                    bool new_value = m_savedUriList.contains(i.value().uri);
            //                    if (m_savedUriList.contains(i.value().uri)) {
            //                        int pos = m_cidList.indexOf(i.value().cid);
            //                        if (pos >= 0) {
            //                            emit dataChanged(index(pos), index(pos),
            //                                             QVector<int>() <<
            //                                             static_cast<int>(SavingRole));
            //                        }
            //                    }
            //                }
        } else {
            emit errorOccured(pref->errorMessage());
        }
        setRunning(false);
        pref->deleteLater();
    });
    pref->setAccount(account());
    pref->getPreferences();
}

void FeedGeneratorListModel::putPreferences(const QString &json)
{
    AppBskyActorPutPreferences *pref = new AppBskyActorPutPreferences(this);
    connect(pref, &AppBskyActorPutPreferences::finished, [=](bool success) {
        if (success) {
            qDebug() << "finish put preferences.";
            getSavedGenerators();
        } else {
            emit errorOccured(pref->errorMessage());
            setRunning(false);
        }
        pref->deleteLater();
    });
    pref->setAccount(account());
    pref->putPreferences(json);
}

QString FeedGeneratorListModel::appendGeneratorToPreference(const QString &src_json,
                                                            const QString &uri) const
{
    if (uri.isEmpty())
        return src_json;
    QJsonDocument json_doc = QJsonDocument::fromJson(src_json.toUtf8());
    QJsonObject root_object = json_doc.object();
    if (root_object.contains("preferences")) {
        QJsonValue preferences = root_object.value("preferences");
        QJsonArray dest_preferences;
        if (preferences.isArray()) {
            for (int i = 0; i < preferences.toArray().count(); i++) {
                if (!preferences.toArray().at(i).isObject())
                    continue;
                QJsonObject value = preferences.toArray().takeAt(i).toObject();
                if (value.value("$type") == QStringLiteral("app.bsky.actor.defs#savedFeedsPref")
                    && value.value("saved").isArray()) {
                    QJsonArray json_saved = value.value("saved").toArray();
                    if (!json_saved.contains(QJsonValue(uri))) {
                        // 含まれていなければ追加
                        json_saved.append(QJsonValue(uri));
                        value.insert("saved", json_saved);
                    }
                }
                dest_preferences.append(value);
            }
            root_object.insert("preferences", dest_preferences);
        }
    }
    json_doc.setObject(root_object);
    return json_doc.toJson(QJsonDocument::Compact);
}

QString FeedGeneratorListModel::removeGeneratorToPreference(const QString &src_json,
                                                            const QString &uri) const
{
    if (uri.isEmpty())
        return src_json;
    QJsonDocument json_doc = QJsonDocument::fromJson(src_json.toUtf8());
    QJsonObject root_object = json_doc.object();
    if (root_object.contains("preferences")) {
        QJsonValue preferences = root_object.value("preferences");
        QJsonArray dest_preferences;
        if (preferences.isArray()) {
            for (int i = 0; i < preferences.toArray().count(); i++) {
                if (!preferences.toArray().at(i).isObject())
                    continue;
                QJsonObject value = preferences.toArray().takeAt(i).toObject();
                if (value.value("$type") == QStringLiteral("app.bsky.actor.defs#savedFeedsPref")
                    && value.value("saved").isArray()) {
                    QJsonArray json_saved;
                    for (const auto &value : value.value("saved").toArray()) {
                        if (value.toString() != uri) {
                            json_saved.append(value);
                        }
                    }
                    value.insert("saved", json_saved);
                }
                dest_preferences.append(value);
            }
            root_object.insert("preferences", dest_preferences);
        }
    }
    json_doc.setObject(root_object);
    return json_doc.toJson(QJsonDocument::Compact);
}

QString FeedGeneratorListModel::query() const
{
    return m_query;
}

void FeedGeneratorListModel::setQuery(const QString &newQuery)
{
    if (m_query == newQuery)
        return;
    m_query = newQuery;
    emit queryChanged();
}
