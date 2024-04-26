#ifndef CREATESESSION_H
#define CREATESESSION_H

#include "atprotocol/accessatprotocol.h"
#include <QObject>

class CreateSession : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString service READ service WRITE setService NOTIFY serviceChanged)
    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(QString authFactorToken READ authFactorToken WRITE setAuthFactorToken NOTIFY
                       authFactorTokenChanged FINAL)

    Q_PROPERTY(QString did READ did WRITE setDid NOTIFY didChanged)
    Q_PROPERTY(QString handle READ handle WRITE setHandle NOTIFY handleChanged)
    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)
    Q_PROPERTY(QString accessJwt READ accessJwt WRITE setAccessJwt NOTIFY accessJwtChanged)
    Q_PROPERTY(QString refreshJwt READ refreshJwt WRITE setRefreshJwt NOTIFY refreshJwtChanged)
    Q_PROPERTY(bool authorized READ authorized WRITE setAuthorized NOTIFY authorizedChanged)

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

public:
    explicit CreateSession(QObject *parent = nullptr);

    Q_INVOKABLE void create();

    QString service() const;
    void setService(const QString &newService);
    QString identifier() const;
    void setIdentifier(const QString &newIdentifier);
    QString password() const;
    void setPassword(const QString &newPassword);
    QString authFactorToken() const;
    void setAuthFactorToken(const QString &newAuthFactorToken);

    QString did() const;
    void setDid(const QString &newDid);
    QString handle() const;
    void setHandle(const QString &newHandle);
    QString email() const;
    void setEmail(const QString &newEmail);
    QString accessJwt() const;
    void setAccessJwt(const QString &newAccessJwt);
    QString refreshJwt() const;
    void setRefreshJwt(const QString &newRefreshJwt);
    bool authorized() const;
    void setAuthorized(bool newAuthorized);

    bool running() const;
    void setRunning(bool newRunning);

signals:
    void errorOccured(const QString &code, const QString &message);
    void finished(bool success);
    void serviceChanged();
    void identifierChanged();
    void passwordChanged();
    void authFactorTokenChanged();

    void didChanged();
    void handleChanged();
    void emailChanged();
    void accessJwtChanged();
    void refreshJwtChanged();
    void authorizedChanged();

    void runningChanged();

private:
    AtProtocolInterface::AccountData m_session;

    QString m_identifier;
    QString m_password;
    bool m_running;
    QString m_authFactorToken;
};

#endif // CREATESESSION_H
