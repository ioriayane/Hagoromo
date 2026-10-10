#ifndef OAUTHLOGIN_H
#define OAUTHLOGIN_H

#include "tools/accountmanager.h"

#include <QObject>
#include <QPointer>

class Authorization;

// ブラウザを使ったOAuthのログイン(QML用)
class OAuthLogin : public QObject
{
    Q_OBJECT

    // ハンドルを解決できないときの問い合わせ先(https://bsky.socialなど)
    Q_PROPERTY(QString service READ service WRITE setService NOTIFY serviceChanged)
    // ハンドルまたはDID
    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    // ブラウザで開く認可URL(開けなかったときにコピーしてもらう)
    Q_PROPERTY(QString authorizationUrl READ authorizationUrl NOTIFY authorizationUrlChanged)

public:
    explicit OAuthLogin(QObject *parent = nullptr);
    ~OAuthLogin();

    Q_INVOKABLE void start();
    Q_INVOKABLE void cancel();

    QString service() const;
    void setService(const QString &newService);
    QString identifier() const;
    void setIdentifier(const QString &newIdentifier);
    bool running() const;
    QString authorizationUrl() const;

    // ログインに成功したセッション
    const OAuthSession &session() const;
    void setSession(const OAuthSession &newSession);

signals:
    void errorOccurred(const QString &code, const QString &message);
    void finished(bool success);
    // QMLでブラウザを開く
    void requestOpenUrl(const QString &url);
    void serviceChanged();
    void identifierChanged();
    void runningChanged();
    void authorizationUrlChanged();

private:
    void setRunning(bool newRunning);
    void setAuthorizationUrl(const QString &newAuthorizationUrl);
    void finish(bool success);

    QPointer<Authorization> m_authorization;
    OAuthSession m_session;
    QString m_service;
    QString m_identifier;
    bool m_running;
    QString m_authorizationUrl;
};

#endif // OAUTHLOGIN_H
