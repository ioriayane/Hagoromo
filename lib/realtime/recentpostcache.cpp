#include "recentpostcache.h"

#include "common.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace RealtimeFeed {

namespace {
const QString RecentPostCacheFileName = QStringLiteral("realtime_recent_post.json");
const int MaxCount = 50;

void trim(QList<OperationInfo> &list)
{
    while (list.count() > MaxCount) {
        list.removeFirst();
    }
}
}

RecentPostCache::RecentPostCache(QObject *parent) : QObject { parent }
{
    load();
    connect(qApp, &QCoreApplication::aboutToQuit, this, &RecentPostCache::save);
}

RecentPostCache::~RecentPostCache() { }

RecentPostCache *RecentPostCache::getInstance()
{
    static RecentPostCache instance;
    return &instance;
}

void RecentPostCache::add(const QString &key, const OperationInfo &info)
{
    if (key.isEmpty())
        return;

    QList<OperationInfo> &list = m_cache[key];
    list.append(info);
    trim(list);
}

QList<OperationInfo> RecentPostCache::get(const QString &key)
{
    if (key.isEmpty())
        return QList<OperationInfo>();
    return m_cache.take(key);
}

void RecentPostCache::save() const
{
    QJsonObject root;
    for (auto it = m_cache.constBegin(); it != m_cache.constEnd(); ++it) {
        QJsonArray array;
        for (const auto &info : it.value()) {
            QJsonObject item;
            item["action"] = static_cast<int>(info.action);
            item["cid"] = info.cid;
            item["uri"] = info.uri;
            item["is_like"] = info.is_like;
            item["is_repost"] = info.is_repost;
            item["reaction_cid"] = info.reaction_cid;
            item["reaction_uri"] = info.reaction_uri;
            item["reacted_by_did"] = info.reacted_by_did;
            item["reacted_by_handle"] = info.reacted_by_handle;
            item["reacted_by_display_name"] = info.reacted_by_display_name;
            item["time"] = info.time;
            array.append(item);
        }
        root[it.key()] = array;
    }
    Common::saveJsonDocument(QJsonDocument(root), RecentPostCacheFileName);
}

void RecentPostCache::load()
{
    QJsonDocument doc = Common::loadJsonDocument(RecentPostCacheFileName);
    if (!doc.isObject())
        return;

    QJsonObject root = doc.object();
    for (auto it = root.constBegin(); it != root.constEnd(); ++it) {
        QList<OperationInfo> list;
        for (const auto &value : it.value().toArray()) {
            QJsonObject item = value.toObject();
            OperationInfo info;
            info.action = static_cast<OperationActionType>(item.value("action").toInt());
            info.cid = item.value("cid").toString();
            info.uri = item.value("uri").toString();
            info.is_like = item.value("is_like").toBool();
            info.is_repost = item.value("is_repost").toBool();
            info.reaction_cid = item.value("reaction_cid").toString();
            info.reaction_uri = item.value("reaction_uri").toString();
            info.reacted_by_did = item.value("reacted_by_did").toString();
            info.reacted_by_handle = item.value("reacted_by_handle").toString();
            info.reacted_by_display_name = item.value("reacted_by_display_name").toString();
            info.time = item.value("time").toString();
            list.append(info);
        }
        trim(list);
        m_cache[it.key()] = list;
    }
}

}
