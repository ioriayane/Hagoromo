#include "atpabstractlistmodel.h"

AtpAbstractListModel::AtpAbstractListModel(QObject *parent)
    : QAbstractListModel { parent }, m_running(false)
{
    connect(&m_timer, &QTimer::timeout, this, &AtpAbstractListModel::getLatest);
}

AtProtocolInterface::AccountData AtpAbstractListModel::account() const
{
    return m_account;
}

void AtpAbstractListModel::setAccount(const QString &service, const QString &did,
                                      const QString &handle, const QString &email,
                                      const QString &accessJwt, const QString &refreshJwt)
{
    m_account.service = service;
    m_account.did = did;
    m_account.handle = handle;
    m_account.email = email;
    m_account.accessJwt = accessJwt;
    m_account.refreshJwt = refreshJwt;
}

bool AtpAbstractListModel::running() const
{
    return m_running;
}

void AtpAbstractListModel::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

QString AtpAbstractListModel::formatDateTime(const QString &value) const
{
    return QDateTime::fromString(value, Qt::ISODateWithMs).toLocalTime().toString("MM/dd hh:mm");
}

bool AtpAbstractListModel::autoLoading() const
{
    return m_timer.isActive();
}

void AtpAbstractListModel::setAutoLoading(bool newAutoLoading)
{
    if (newAutoLoading) {
        // Off -> On
        if (m_timer.isActive())
            return;
        m_timer.start(60 * 1000);
    } else {
        // * -> Off
        m_timer.stop();
    }
    emit autoLoadingChanged();
}