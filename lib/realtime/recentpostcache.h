#ifndef RECENTPOSTCACHE_H
#define RECENTPOSTCACHE_H

#include "realtime/abstractpostselector.h"

#include <QHash>
#include <QObject>

namespace RealtimeFeed {

class RecentPostCache : public QObject
{
    Q_OBJECT

    RecentPostCache(QObject *parent = nullptr);
    ~RecentPostCache();

public:
    RecentPostCache(const RecentPostCache &) = delete;
    RecentPostCache &operator=(const RecentPostCache &) = delete;
    RecentPostCache(RecentPostCache &&) = delete;
    RecentPostCache &operator=(RecentPostCache &&) = delete;

    static RecentPostCache *getInstance();

    // key(カラムごとのキー)ごとに保存する。50件を超えたら古いものから削除する
    void add(const QString &key, const OperationInfo &info);
    // keyに紐づく保存内容を取り出す(取り出した内容はキャッシュから取り除かれる)
    QList<OperationInfo> get(const QString &key);

    void save() const;
    void load();

private:
    QHash<QString, QList<OperationInfo>> m_cache;
};

}

#endif // RECENTPOSTCACHE_H
