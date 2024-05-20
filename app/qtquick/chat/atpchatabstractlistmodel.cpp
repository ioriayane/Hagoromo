#include "atpchatabstractlistmodel.h"

AtpChatAbstractListModel::AtpChatAbstractListModel(QObject *parent)
    : QAbstractListModel { parent }, m_running(false)
{
}

void AtpChatAbstractListModel::clear()
{
    //
}

AtProtocolInterface::AccountData AtpChatAbstractListModel::account() const
{
    return m_account;
}

void AtpChatAbstractListModel::setAccount(const QString &service, const QString &did,
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

bool AtpChatAbstractListModel::running() const
{
    return m_running;
}

void AtpChatAbstractListModel::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}
