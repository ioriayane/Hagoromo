#ifndef PINNEDPOSTCACHE_H
#define PINNEDPOSTCACHE_H

#include <QHash>
#include <QObject>

class PinnedPostCache : public QObject
{
    Q_OBJECT

    PinnedPostCache(QObject *parent = nullptr);
    ~PinnedPostCache();

public:
    PinnedPostCache(const PinnedPostCache &) = delete;
    PinnedPostCache &operator=(const PinnedPostCache &) = delete;
    PinnedPostCache(PinnedPostCache &&) = delete;
    PinnedPostCache &operator=(PinnedPostCache &&) = delete;

    static PinnedPostCache *getInstance();

    void update(const QString &did, const QString &uri);
    bool pinned(const QString &did, const QString &uri) const;

signals:
    void updated(const QString &did, const QString &new_uri, const QString &old_uri);

private:
    QHash<QString, QString> m_pinnedPost;
};

#endif // PINNEDPOSTCACHE_H
