#ifndef DPOPSESSIONSTORE_H
#define DPOPSESSIONSTORE_H

#include <QByteArray>
#include <QHash>
#include <QMutex>
#include <QString>
#include <QUrl>

class Es256;

// OAuthのアカウントごとのDPoPの鍵と、送信先(origin)ごとのnonceを保持する
// AccountDataは各リクエストにコピーされるので、鍵はアカウントのuuidで引く
class DPopSessionStore
{
    explicit DPopSessionStore();
    ~DPopSessionStore();

public:
    static DPopSessionStore *getInstance();

    bool setPrivateKey(const QString &uuid, const QByteArray &pem);
    bool hasSession(const QString &uuid) const;
    void removeSession(const QString &uuid);
    void clear();

    // DPoP proof(access_tokenを指定するとathを付ける)
    QByteArray generateProof(const QString &uuid, const QString &method, const QUrl &url,
                             const QString &access_token = QString()) const;
    QString nonce(const QString &uuid, const QUrl &url) const;
    void setNonce(const QString &uuid, const QUrl &url, const QString &nonce);

    static QString origin(const QUrl &url);

private:
    Q_DISABLE_COPY(DPopSessionStore)

    struct Session
    {
        Es256 *key;
        QHash<QString, QString> nonces; // origin -> nonce
    };

    mutable QMutex m_mutex;
    QHash<QString, Session> m_sessions;
};

#endif // DPOPSESSIONSTORE_H
