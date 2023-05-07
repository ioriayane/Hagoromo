#include "createsession.h"

#include <QDebug>

using AtProtocolInterface::AccountData;
using AtProtocolInterface::AccountStatus;
using AtProtocolInterface::ComAtprotoServerCreateSession;

CreateSession::CreateSession(QObject *parent) : QObject { parent }, m_running(false) { }

void CreateSession::create()
{
    if (running()) {
        qDebug() << "Already running 'ComAtprotoServerCreateSession()'.";
    }

    setRunning(true);
    ComAtprotoServerCreateSession *session = new ComAtprotoServerCreateSession();
    connect(session, &ComAtprotoServerCreateSession::finished, [=](bool success) {
        setDid(session->did());
        setHandle(session->handle());
        setEmail(session->email());
        setAccessJwt(session->accessJwt());
        setRefreshJwt(session->refreshJwt());
        setAuthorized(success);
        emit finished(success);
        setRunning(false);
        session->deleteLater();
    });
    session->setService(service());
    session->create(identifier(), password());
}

QString CreateSession::service() const
{
    return m_session.service;
}

void CreateSession::setService(const QString &newService)
{
    if (m_session.service == newService)
        return;
    m_session.service = newService;
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
    return m_session.did;
}

void CreateSession::setDid(const QString &newDid)
{
    if (m_session.did == newDid)
        return;
    m_session.did = newDid;
    emit didChanged();
}

QString CreateSession::handle() const
{
    return m_session.handle;
}

void CreateSession::setHandle(const QString &newHandle)
{
    if (m_session.handle == newHandle)
        return;
    m_session.handle = newHandle;
    emit handleChanged();
}

QString CreateSession::email() const
{
    return m_session.email;
}

void CreateSession::setEmail(const QString &newEmail)
{
    if (m_session.email == newEmail)
        return;
    m_session.email = newEmail;
    emit emailChanged();
}

QString CreateSession::accessJwt() const
{
    return m_session.accessJwt;
}

void CreateSession::setAccessJwt(const QString &newAccessJwt)
{
    if (m_session.accessJwt == newAccessJwt)
        return;
    m_session.accessJwt = newAccessJwt;
    emit accessJwtChanged();
}

QString CreateSession::refreshJwt() const
{
    return m_session.refreshJwt;
}

void CreateSession::setRefreshJwt(const QString &newRefreshJwt)
{
    if (m_session.refreshJwt == newRefreshJwt)
        return;
    m_session.refreshJwt = newRefreshJwt;
    emit refreshJwtChanged();
}

bool CreateSession::authorized() const
{
    return (m_session.status == AccountStatus::Authorized);
}

void CreateSession::setAuthorized(bool newAuthorized)
{
    if ((m_session.status == AccountStatus::Authorized) == newAuthorized)
        return;
    m_session.status = newAuthorized ? AccountStatus::Authorized : AccountStatus::Unauthorized;
    emit authorizedChanged();
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
