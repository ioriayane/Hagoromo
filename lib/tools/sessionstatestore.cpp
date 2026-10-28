#include "sessionstatestore.h"

#include <QMutexLocker>

SessionStateStore::SessionStateStore() { }

SessionStateStore::~SessionStateStore() { }

SessionStateStore *SessionStateStore::getInstance()
{
    static SessionStateStore instance;
    return &instance;
}

void SessionStateStore::setReauthRequired(const QString &uuid)
{
    if (uuid.isEmpty()) {
        return;
    }
    QMutexLocker locker(&m_mutex);
    m_reauthRequired.insert(uuid);
}

bool SessionStateStore::isReauthRequired(const QString &uuid) const
{
    if (uuid.isEmpty()) {
        return false;
    }
    QMutexLocker locker(&m_mutex);
    return m_reauthRequired.contains(uuid);
}

void SessionStateStore::remove(const QString &uuid)
{
    QMutexLocker locker(&m_mutex);
    m_reauthRequired.remove(uuid);
}

void SessionStateStore::clear()
{
    QMutexLocker locker(&m_mutex);
    m_reauthRequired.clear();
}
