#ifndef SESSIONSTATESTORE_H
#define SESSIONSTATESTORE_H

#include <QDateTime>
#include <QHash>
#include <QMutex>
#include <QSet>
#include <QString>

#include <functional>

// アカウントごとのセッションの状態を保持する
// AccountDataは各リクエストにコピーされて古くなるので、送信直前にアカウントのuuidで引く
class SessionStateStore
{
    explicit SessionStateStore();
    ~SessionStateStore();

public:
    static SessionStateStore *getInstance();

    // 再ログインするまでリクエストは必ず失敗する
    void setReauthRequired(const QString &uuid);
    bool isReauthRequired(const QString &uuid) const;

    // アクセストークンの期限(期限が分からないときは無効な値で、期限切れとして扱わない)
    void setAccessTokenExpiresAt(const QString &uuid, const QDateTime &expires_at);
    bool isAccessTokenExpired(const QString &uuid) const;
    // リフレッシュを依頼する(同じアカウントの依頼は一定時間に1回だけ)
    bool requestRefresh(const QString &uuid);
    void setRefreshRequestHandler(std::function<void(const QString &uuid)> handler);

    void remove(const QString &uuid);
    void clear();

private:
    Q_DISABLE_COPY(SessionStateStore)

    mutable QMutex m_mutex;
    QSet<QString> m_reauthRequired;
    QHash<QString, QDateTime> m_accessTokenExpiresAt;
    QHash<QString, QDateTime> m_refreshRequestedAt;
    std::function<void(const QString &uuid)> m_refreshRequestHandler;
};

#endif // SESSIONSTATESTORE_H
