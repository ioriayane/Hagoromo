#include "sessionstatestore.h"

#include <QMutexLocker>

// サーバーで発行されてから受け取るまでの時間を考慮して、少し早めに期限切れとする
#define ACCESS_TOKEN_EXPIRATION_MARGIN_SEC 10
// リフレッシュの依頼の間隔(失敗したときに繰り返さないため)
#define REFRESH_REQUEST_INTERVAL_SEC 60

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

void SessionStateStore::setAccessTokenExpiresAt(const QString &uuid, const QDateTime &expires_at)
{
    if (uuid.isEmpty()) {
        return;
    }
    QMutexLocker locker(&m_mutex);
    if (expires_at.isValid()) {
        m_accessTokenExpiresAt.insert(uuid, expires_at);
    } else {
        m_accessTokenExpiresAt.remove(uuid);
    }
    // 新しいトークンになったので、次に期限が切れたらすぐに依頼できるようにする
    m_refreshRequestedAt.remove(uuid);
}

bool SessionStateStore::isAccessTokenExpired(const QString &uuid) const
{
    if (uuid.isEmpty()) {
        return false;
    }
    QMutexLocker locker(&m_mutex);
    if (!m_accessTokenExpiresAt.contains(uuid)) {
        return false;
    }
    return QDateTime::currentDateTimeUtc().addSecs(ACCESS_TOKEN_EXPIRATION_MARGIN_SEC)
            >= m_accessTokenExpiresAt.value(uuid);
}

bool SessionStateStore::requestRefresh(const QString &uuid)
{
    if (uuid.isEmpty()) {
        return false;
    }
    std::function<void(const QString &uuid)> handler;
    {
        QMutexLocker locker(&m_mutex);
        if (!m_refreshRequestHandler) {
            return false;
        }
        const QDateTime now = QDateTime::currentDateTimeUtc();
        if (m_refreshRequestedAt.contains(uuid)
            && m_refreshRequestedAt.value(uuid).secsTo(now) < REFRESH_REQUEST_INTERVAL_SEC) {
            return false;
        }
        m_refreshRequestedAt.insert(uuid, now);
        handler = m_refreshRequestHandler;
    }
    // ハンドラーの中でこのクラスを使っても良いようにロックの外で呼ぶ
    handler(uuid);
    return true;
}

void SessionStateStore::setRefreshRequestHandler(std::function<void(const QString &)> handler)
{
    QMutexLocker locker(&m_mutex);
    m_refreshRequestHandler = handler;
}

void SessionStateStore::remove(const QString &uuid)
{
    QMutexLocker locker(&m_mutex);
    m_reauthRequired.remove(uuid);
    m_accessTokenExpiresAt.remove(uuid);
    m_refreshRequestedAt.remove(uuid);
}

void SessionStateStore::clear()
{
    QMutexLocker locker(&m_mutex);
    m_reauthRequired.clear();
    m_accessTokenExpiresAt.clear();
    m_refreshRequestedAt.clear();
}
