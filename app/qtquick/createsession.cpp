#include "createsession.h"

#include <QDebug>

CreateSession::CreateSession(QObject *parent) : QObject { parent }, m_running(false)
{
    //    connect(&m_session, &ComAtprotoServerCreateSession::finished, this,
    //    &CreateSession::finished);
    connect(&m_session, &ComAtprotoServerCreateSession::finished, [=](bool success) {
        emit didChanged();
        emit handleChanged();
        emit emailChanged();
        emit accessJwtChanged();
        emit refreshJwtChanged();
        emit finished(success);
        setRunning(false);
    });
}

void CreateSession::create()
{
    setRunning(true);
    m_session.setService(service());
    m_session.create(identifier(), password());
}

QString CreateSession::service() const
{
    return m_session.service();
}

void CreateSession::setService(const QString &newService)
{
    if (m_session.service() == newService)
        return;
    m_session.setService(newService);
    emit serviceChanged();
}

QString CreateSession::identifier() const
{
    return m_identifier;
}

void CreateSession::setIdentifier(const QString &newIdentifier)
{
    if (m_identifier == newIdentifier)
        return;
    m_identifier = newIdentifier;
    emit identifierChanged();
}

QString CreateSession::password() const
{
    return m_password;
}

void CreateSession::setPassword(const QString &newPassword)
{
    if (m_password == newPassword)
        return;
    m_password = newPassword;
    emit passwordChanged();
}

QString CreateSession::did() const
{
    return m_session.did();
}

QString CreateSession::handle() const
{
    return m_session.handle();
}

QString CreateSession::email() const
{
    return m_session.email();
}

QString CreateSession::accessJwt() const
{
    return m_session.accessJwt();
}

QString CreateSession::refreshJwt() const
{
    return m_session.refreshJwt();
}

bool CreateSession::running() const
{
    return m_running;
}

void CreateSession::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}
