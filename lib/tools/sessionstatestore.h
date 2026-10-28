#ifndef SESSIONSTATESTORE_H
#define SESSIONSTATESTORE_H

#include <QMutex>
#include <QSet>
#include <QString>

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
    void remove(const QString &uuid);
    void clear();

private:
    Q_DISABLE_COPY(SessionStateStore)

    mutable QMutex m_mutex;
    QSet<QString> m_reauthRequired;
};

#endif // SESSIONSTATESTORE_H
