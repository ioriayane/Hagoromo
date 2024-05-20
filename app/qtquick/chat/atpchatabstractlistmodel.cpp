#include "atpchatabstractlistmodel.h"
#include "extension/directory/plc/directoryplc.h"

using AtProtocolInterface::DirectoryPlc;

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

void AtpChatAbstractListModel::setServiceEndpoint(const QString &service_endpoint)
{
    m_account.service_endpoint = service_endpoint;
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

void AtpChatAbstractListModel::getServiceEndpoint(std::function<void()> callback)
{
    if (!m_account.service_endpoint.isEmpty()) {
        callback();
        return;
    }
    if (account().did.isEmpty()) {
        callback();
        return;
    }

    DirectoryPlc *plc = new DirectoryPlc(this);
    connect(plc, &DirectoryPlc::finished, this, [=](bool success) {
        if (success && !plc->serviceEndpoint().isEmpty()) {
            m_account.service_endpoint = plc->serviceEndpoint();
        } else {
            m_account.service_endpoint = m_account.service;
        }
        callback();
        plc->deleteLater();
    });
    plc->directory(account().did);
}
