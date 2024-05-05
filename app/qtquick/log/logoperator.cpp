#include "logoperator.h"
#include "log/logmanager.h"

LogOperator::LogOperator(QObject *parent) : QObject { parent }, m_running(false) { }

void LogOperator::getLatest()
{
    if (running() || service().isEmpty() || did().isEmpty())
        return;
    setRunning(true);

    LogManager *manager = new LogManager(this);
    connect(manager, &LogManager::finished, this, [=](bool success) {
        if (success) {
            //
        }
        emit finished(success);
        setRunning(false);
        manager->deleteLater();
    });
    manager->update(service(), did());
}

void LogOperator::clear()
{
    if (did().isEmpty())
        return;

    LogManager manager;
    manager.clearDb(did());
}

bool LogOperator::running() const
{
    return m_running;
}

void LogOperator::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

QString LogOperator::service() const
{
    return m_service;
}

void LogOperator::setService(const QString &newService)
{
    if (m_service == newService)
        return;
    m_service = newService;
    emit serviceChanged();
}

QString LogOperator::did() const
{
    return m_did;
}

void LogOperator::setDid(const QString &newDid)
{
    if (m_did == newDid)
        return;
    m_did = newDid;
    emit didChanged();
}
