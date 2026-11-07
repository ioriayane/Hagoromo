#include "pinnedpostcache.h"

#include <QDebug>

PinnedPostCache::PinnedPostCache(QObject *parent) : QObject { parent }
{
    qDebug() << "PinnedPostCache()";
}

PinnedPostCache::~PinnedPostCache()
{
    qDebug() << "~PinnedPostCache()";
}

PinnedPostCache *PinnedPostCache::getInstance()
{
    static PinnedPostCache instance;
    return &instance;
}

void PinnedPostCache::update(const QString &did, const QString &uri)
{
    if (m_pinnedPost.value(did) == uri)
        return;
    QString old_uri = m_pinnedPost.value(did);
    m_pinnedPost[did] = uri;

    emit updated(did, uri, old_uri);
}

bool PinnedPostCache::pinned(const QString &did, const QString &uri) const
{
    return (!uri.isEmpty() && m_pinnedPost.value(did, QString()) == uri);
}
