#include "atpchatabstractlistmodel.h"
#include "extension/directory/plc/directoryplc.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvoupdateread.h"
#include "tools/labelerprovider.h"
#include "tools/accountmanager.h"

using AtProtocolInterface::ChatBskyConvoUpdateRead;
using AtProtocolInterface::DirectoryPlc;

AtpChatAbstractListModel::AtpChatAbstractListModel(QObject *parent)
    : QAbstractListModel { parent }, m_running(false), m_loadingInterval(30000)
{
    connect(&m_timer, &QTimer::timeout, this, &AtpChatAbstractListModel::getLatest);
}

void AtpChatAbstractListModel::clear()
{
    //
}

AtProtocolInterface::AccountData AtpChatAbstractListModel::account() const
{
    return AccountManager::getInstance()->getAccount(m_account.uuid);
}

void AtpChatAbstractListModel::setAccount(const QString &uuid)
{
    m_account.uuid = uuid;
}

void AtpChatAbstractListModel::updateRead(const QString &convoId, const QString &messageId)
{
    if (convoId.isEmpty())
        return;

    ChatBskyConvoUpdateRead *read = new ChatBskyConvoUpdateRead(this);
    connect(read, &ChatBskyConvoUpdateRead::finished, this, [=](bool success) {
        if (success) {
            qDebug() << "updateRead" << read->convo().unreadCount;
        } else {
            emit errorOccured(read->errorCode(), read->errorMessage());
        }
        emit finishUpdateRead(success);
        read->deleteLater();
    });
    read->setAccount(account());
    read->setService(account().service_endpoint);
    read->updateRead(convoId, messageId);
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
    if (!account().service_endpoint.isEmpty()) {
        callback();
        return;
    }
    if (account().did.isEmpty()) {
        callback();
        return;
    }
    if (!account().service.startsWith("https://bsky.social")) {
        account().service_endpoint = account().service;
        qDebug().noquote() << "Update service endpoint(chat)" << account().service << "->"
                           << account().service_endpoint;
        callback();
        return;
    }

    DirectoryPlc *plc = new DirectoryPlc(this);
    connect(plc, &DirectoryPlc::finished, this, [=](bool success) {
        QString service_endpoint = account().service;
        if (success && !plc->serviceEndpoint().isEmpty()) {
            service_endpoint = plc->serviceEndpoint();
        }
        AccountManager::getInstance()->updateServiceEndpoint(account().uuid, service_endpoint);
        qDebug().noquote() << "Update service endpoint(chat)" << account().service << "->"
                           << account().service_endpoint;
        callback();
        plc->deleteLater();
    });
    plc->directory(account().did);
}

void AtpChatAbstractListModel::checkScopeError(const QString &code, const QString &message)
{
    Q_UNUSED(message)
    if (code == "InvalidToken") {
        setAutoLoading(false);
    }
}

void AtpChatAbstractListModel::updateContentFilterLabels(std::function<void()> callback)
{
    LabelerProvider *provider = LabelerProvider::getInstance();
    LabelerConnector *connector = new LabelerConnector(this);

    connect(connector, &LabelerConnector::finished, this, [=](bool success) {
        if (success) { }
        callback();
        connector->deleteLater();
    });
    provider->setAccount(account());
    provider->update(account(), connector, LabelerProvider::RefleshAuto);
}

QStringList AtpChatAbstractListModel::labelerDids() const
{
    return LabelerProvider::getInstance()->labelerDids(account());
}

ConfigurableLabelStatus AtpChatAbstractListModel::getContentFilterStatus(
        const QList<AtProtocolType::ComAtprotoLabelDefs::Label> &labels, const bool for_media) const
{
    return LabelerProvider::getInstance()->getContentFilterStatus(account(), labels, for_media);
}

bool AtpChatAbstractListModel::autoLoading() const
{
    return m_timer.isActive();
}

void AtpChatAbstractListModel::setAutoLoading(bool newAutoLoading)
{
    qDebug() << __FUNCTION__ << "new=" << newAutoLoading << ", active=" << m_timer.isActive()
             << ", interval=" << loadingInterval();
    if (newAutoLoading) {
        // Off -> On
        if (m_timer.isActive())
            return;
        m_timer.start(loadingInterval());
    } else {
        // * -> Off
        m_timer.stop();
    }
    emit autoLoadingChanged();
}

int AtpChatAbstractListModel::loadingInterval() const
{
    return m_loadingInterval;
}

void AtpChatAbstractListModel::setLoadingInterval(int newLoadingInterval)
{
    if (m_loadingInterval == newLoadingInterval)
        return;
    m_loadingInterval = newLoadingInterval;
    m_timer.setInterval(m_loadingInterval);
    emit loadingIntervalChanged();
}
