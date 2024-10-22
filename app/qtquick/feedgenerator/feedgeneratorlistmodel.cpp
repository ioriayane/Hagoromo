#include "feedgeneratorlistmodel.h"

#include "atprotocol/app/bsky/actor/appbskyactorgetpreferences.h"
#include "atprotocol/app/bsky/unspecced/appbskyunspeccedgetpopularfeedgenerators.h"
#include "atprotocol/app/bsky/actor/appbskyactorputpreferences.h"
#include "tools/tid.h"

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

QString FeedGeneratorListModel::getOfficialUrl() const
{
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

bool FeedGeneratorListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    clear();

    AppBskyUnspeccedGetPopularFeedGenerators *generators =
            new AppBskyUnspeccedGetPopularFeedGenerators(this);
    connect(generators, &AppBskyUnspeccedGetPopularFeedGenerators::finished, [=](bool success) {
        if (success && !generators->feedsList().isEmpty()) {
            beginInsertRows(QModelIndex(), 0, generators->feedsList().count() - 1);
            for (const auto &generator : generators->feedsList()) {
                m_cidList.append(generator.cid);
                m_generatorViewHash[generator.cid] = generator;
            }
            endInsertRows();

            m_cursor = generators->cursor();
            getSavedGenerators();
        } else {
            emit errorOccured(generators->errorCode(), generators->errorMessage());
            setRunning(false);
        }
        generators->deleteLater();
    });
    generators->setAccount(account());
    generators->getPopularFeedGenerators(50, QString(), query());

    return true;
}

bool FeedGeneratorListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    AppBskyUnspeccedGetPopularFeedGenerators *generators =
            new AppBskyUnspeccedGetPopularFeedGenerators(this);
    connect(generators, &AppBskyUnspeccedGetPopularFeedGenerators::finished, [=](bool success) {
        if (success && !generators->feedsList().isEmpty()) {
            beginInsertRows(QModelIndex(), m_cidList.count(),
                            m_cidList.count() + generators->feedsList().count() - 1);
            for (const auto &generator : generators->feedsList()) {
                m_cidList.append(generator.cid);
                m_generatorViewHash[generator.cid] = generator;
            }
            endInsertRows();

            m_cursor = generators->cursor();
            // getSavedGenerators();
        } else {
            m_cursor.clear();
            emit errorOccured(generators->errorCode(), generators->errorMessage());
        }
        setRunning(false);
        generators->deleteLater();
    });
    generators->setAccount(account());
    generators->getPopularFeedGenerators(50, m_cursor, query());

    return true;
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
            emit errorOccured(pref->errorCode(), pref->errorMessage());
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
            emit errorOccured(pref->errorCode(), pref->errorMessage());
            setRunning(false);
        }
        pref->deleteLater();
    });
    pref->setAccount(account());
    pref->getPreferences();
}

bool FeedGeneratorListModel::getSaving(const QString &uri) const
{
    return m_savedUriList.contains(uri);
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
            for (const auto &prefs : pref->preferences().savedFeedsPrefV2) {
                for (const auto &item : prefs.items) {
                    if (item.type == "feed") {
                        m_savedUriList.append(item.value);
                    }
                }
            }
            for (const auto &feed : pref->preferences().savedFeedsPref) {
                for (const auto &uri : feed.saved) {
                    if (!m_savedUriList.contains(uri)) {
                        m_savedUriList.append(uri);
                    }
                }
            }
            emit dataChanged(index(0), index(m_cidList.count() - 1),
                             QVector<int>() << static_cast<int>(SavingRole));
        } else {
            emit errorOccured(pref->errorCode(), pref->errorMessage());
        }
        setRunning(false);
        pref->deleteLater();
    });
    pref->setAccount(account());
    pref->getPreferences();
}

void FeedGeneratorListModel::putPreferences(const QJsonArray &json)
{
    AppBskyActorPutPreferences *pref = new AppBskyActorPutPreferences(this);
    connect(pref, &AppBskyActorPutPreferences::finished, [=](bool success) {
        if (success) {
            qDebug() << "finish put preferences.";
            getSavedGenerators();
        } else {
            emit errorOccured(pref->errorCode(), pref->errorMessage());
            setRunning(false);
        }
        pref->deleteLater();
    });
    pref->setAccount(account());
    pref->putPreferences(json);
}

QJsonArray FeedGeneratorListModel::appendGeneratorToPreference(const QString &src_json,
                                                               const QString &uri) const
{
    if (uri.isEmpty())
        return QJsonArray();
    QJsonDocument json_doc = QJsonDocument::fromJson(src_json.toUtf8());
    QJsonObject root_object = json_doc.object();
    if (!root_object.contains("preferences"))
        return QJsonArray();

    QJsonValue preferences = root_object.value("preferences");
    QJsonArray dest_preferences;
    if (!preferences.isArray())
        return QJsonArray();
    for (int i = 0; i < preferences.toArray().count(); i++) {
        if (!preferences.toArray().at(i).isObject())
            continue;
        QJsonObject value = preferences.toArray().takeAt(i).toObject();
        if (value.value("$type") == QStringLiteral("app.bsky.actor.defs#savedFeedsPrefV2")
            && value.value("items").isArray()) {
            QJsonArray json_items = value.value("items").toArray();
            QJsonArray json_items_dest;
            bool exist = false;
            for (const auto &v : qAsConst(json_items)) {
                if (v.toObject().value("value").toString() == uri) {
                    exist = true;
                }
                json_items_dest.append(v);
            }
            if (!exist) {
                // 含まれていなければ追加
                QJsonObject json_item;
                json_item.insert("id", Tid::next());
                json_item.insert("pinned", false);
                json_item.insert("type", "feed");
                json_item.insert("value", uri);
                json_items_dest.append(json_item);
            }
            value.insert("items", json_items_dest);
        }
        dest_preferences.append(value);
    }
    return dest_preferences;
}

QJsonArray FeedGeneratorListModel::removeGeneratorToPreference(const QString &src_json,
                                                               const QString &uri) const
{
    if (uri.isEmpty())
        return QJsonArray();
    QJsonDocument json_doc = QJsonDocument::fromJson(src_json.toUtf8());
    QJsonObject root_object = json_doc.object();
    if (!root_object.contains("preferences"))
        return QJsonArray();
    QJsonValue preferences = root_object.value("preferences");
    QJsonArray dest_preferences;
    if (!preferences.isArray())
        return QJsonArray();
    for (int i = 0; i < preferences.toArray().count(); i++) {
        if (!preferences.toArray().at(i).isObject())
            continue;
        QJsonObject value = preferences.toArray().takeAt(i).toObject();
        if (value.value("$type") == QStringLiteral("app.bsky.actor.defs#savedFeedsPrefV2")
            && value.value("items").isArray()) {
            QJsonArray json_items = value.value("items").toArray();
            QJsonArray json_items_dest;
            for (const auto &v : qAsConst(json_items)) {
                if (v.toObject().value("value").toString() != uri) {
                    json_items_dest.append(v);
                }
            }
            value.insert("items", json_items_dest);
        }
        dest_preferences.append(value);
    }
    return dest_preferences;
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
