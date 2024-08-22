#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <QObject>

class Authorization : public QObject
{
    Q_OBJECT
public:
    explicit Authorization(QObject *parent = nullptr);

    void reset();
    void makeClientId();
    void makeCodeChallenge();
    void makeParPayload();
    void par();
    void authorization(const QString &request_uri);
    void startRedirectServer();

    void makeRequestTokenPayload();
    bool requestToken();

    QByteArray codeVerifier() const;
    QByteArray codeChallenge() const;
    QByteArray ParPayload() const;

signals:
    void finished(bool success);

private:
    QByteArray generateRandomValues() const;
    QString simplyEncode(QString text) const;

    QString m_redirectUri;
    QString m_clientId;
    // par
    QByteArray m_codeChallenge;
    QByteArray m_codeVerifier;
    QByteArray m_state;
    QByteArray m_parPayload;
    // request token
    QByteArray m_code;
    QByteArray m_requestTokenPayload;

    QString m_listenPort;
};

#endif // AUTHORIZATION_H
