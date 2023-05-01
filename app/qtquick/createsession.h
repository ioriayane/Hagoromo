#ifndef CREATESESSION_H
#define CREATESESSION_H

#include "../atprotocol/comatprotoservercreatesession.h"
#include <QObject>

class CreateSession : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString service READ service WRITE setService NOTIFY serviceChanged)
    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)

    Q_PROPERTY(QString did READ did NOTIFY didChanged)
    Q_PROPERTY(QString handle READ handle NOTIFY handleChanged)
    Q_PROPERTY(QString email READ email NOTIFY emailChanged)
    Q_PROPERTY(QString accessJwt READ accessJwt NOTIFY accessJwtChanged)
    Q_PROPERTY(QString refreshJwt READ refreshJwt NOTIFY refreshJwtChanged)

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
    QString did() const;
    QString handle() const;
    QString email() const;
    QString accessJwt() const;
    QString refreshJwt() const;

    bool running() const;
    void setRunning(bool newRunning);

signals:
    void finished(bool success);
    void serviceChanged();
    void identifierChanged();
    void passwordChanged();
    void didChanged();
    void handleChanged();
    void emailChanged();
    void accessJwtChanged();
    void refreshJwtChanged();

    void runningChanged();

private:
    ComAtprotoServerCreateSession m_session;

    QString m_identifier;
    QString m_password;
    bool m_running;
};

#endif // CREATESESSION_H
