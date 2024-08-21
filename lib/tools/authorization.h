#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <QObject>

class Authorization : public QObject
{
    Q_OBJECT
public:
    explicit Authorization(QObject *parent = nullptr);

    void reset();
    void makeCodeChallenge();
    void makeParPayload();
    void par();
    void authorization(const QString &request_uri);
    void startRedirectServer();
    void requestToken();

    QByteArray codeVerifier() const;
    QByteArray codeChallenge() const;
    QByteArray ParPlayload() const;

signals:
    void finished(bool success);

private:
    QByteArray generateRandomValues() const;
    QString simplyEncode(QString text) const;

    QByteArray m_codeChallenge;
    QByteArray m_codeVerifier;
    QByteArray m_state;
    QByteArray m_parPlayload;

    QString m_listenPort;
};

#endif // AUTHORIZATION_H
