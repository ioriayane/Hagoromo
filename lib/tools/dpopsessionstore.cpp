#include "dpopsessionstore.h"
#include "tools/es256.h"
#include "tools/jsonwebtoken.h"

#include <QMutexLocker>

DPopSessionStore::DPopSessionStore() { }

DPopSessionStore::~DPopSessionStore()
{
    clear();
}

DPopSessionStore *DPopSessionStore::getInstance()
{
    static DPopSessionStore instance;
    return &instance;
}

bool DPopSessionStore::setPrivateKey(const QString &uuid, const QByteArray &pem)
{
    if (uuid.isEmpty()) {
        return false;
    }
    Es256 *key = new Es256();
    if (!key->loadPrivateKeyPem(pem)) {
        delete key;
        return false;
    }

    QMutexLocker locker(&m_mutex);
    if (m_sessions.contains(uuid)) {
        // 鍵が変わったら以前のnonceは使わない
        delete m_sessions.value(uuid).key;
    }
    Session session;
    session.key = key;
    m_sessions.insert(uuid, session);
    return true;
}

bool DPopSessionStore::hasSession(const QString &uuid) const
{
    QMutexLocker locker(&m_mutex);
    return m_sessions.contains(uuid);
}

void DPopSessionStore::removeSession(const QString &uuid)
{
    QMutexLocker locker(&m_mutex);
    if (m_sessions.contains(uuid)) {
        delete m_sessions.value(uuid).key;
        m_sessions.remove(uuid);
    }
}

void DPopSessionStore::clear()
{
    QMutexLocker locker(&m_mutex);
    for (const auto &session : std::as_const(m_sessions)) {
        delete session.key;
    }
    m_sessions.clear();
}

QByteArray DPopSessionStore::generateProof(const QString &uuid, const QString &method,
                                           const QUrl &url, const QString &access_token) const
{
    QMutexLocker locker(&m_mutex);
    if (!m_sessions.contains(uuid)) {
        return QByteArray();
    }
    const Session &session = m_sessions[uuid];
    return JsonWebToken::generate(*session.key, url.toString(), method,
                                  session.nonces.value(origin(url)), access_token);
}

QString DPopSessionStore::nonce(const QString &uuid, const QUrl &url) const
{
    QMutexLocker locker(&m_mutex);
    if (!m_sessions.contains(uuid)) {
        return QString();
    }
    return m_sessions[uuid].nonces.value(origin(url));
}

void DPopSessionStore::setNonce(const QString &uuid, const QUrl &url, const QString &nonce)
{
    QMutexLocker locker(&m_mutex);
    if (nonce.isEmpty() || !m_sessions.contains(uuid)) {
        return;
    }
    m_sessions[uuid].nonces.insert(origin(url), nonce);
}

QString DPopSessionStore::origin(const QUrl &url)
{
    return url
            .adjusted(QUrl::RemoveUserInfo | QUrl::RemovePath | QUrl::RemoveQuery
                      | QUrl::RemoveFragment)
            .toString();
}
