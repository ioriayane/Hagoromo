#include "logmanager.h"
#include "atprotocol/com/atproto/sync/comatprotosyncgetrepo.h"

#include <QDebug>

using AtProtocolInterface::ComAtprotoSyncGetRepo;

LogManager::LogManager(QObject *parent) : QObject { parent }
{
    m_logAccess.moveToThread(&m_thread);
    connect(this, &LogManager::updateDb, &m_logAccess, &LogAccess::updateDb);
    connect(this, &LogManager::dailyTotals, &m_logAccess, &LogAccess::dailyTotals);
    connect(this, &LogManager::monthlyTotals, &m_logAccess, &LogAccess::monthlyTotals);
    connect(this, &LogManager::statistics, &m_logAccess, &LogAccess::statistics);
    connect(this, &LogManager::selectRecords, &m_logAccess, &LogAccess::selectRecords);
    connect(&m_logAccess, &LogAccess::finishedUpdateDb, this,
            [=](bool success) { emit finished(success); });
    connect(&m_logAccess, &LogAccess::finishedTotals, this,
            [=](const QList<TotalItem> &list) { emit finishedTotals(list); });
    connect(&m_logAccess, &LogAccess::finishedSelection, this,
            [=](const QString &records) { emit finishedSelection(records); });
    m_thread.start();
}

LogManager::~LogManager()
{
    m_thread.exit();
    m_thread.wait();
}

void LogManager::update(const QString &service, const QString &did)
{
    qDebug() << "update" << service << did;

    if (service.isEmpty() || did.isEmpty()) {
        emit finished(false);
        return;
    }

    ComAtprotoSyncGetRepo *repo = new ComAtprotoSyncGetRepo(this);
    connect(repo, &ComAtprotoSyncGetRepo::finished, this, [=](bool success) {
        if (success) {
            emit updateDb(did, repo->repo());
        } else {
            emit finished(false);
        }
        repo->deleteLater();
    });
    repo->setService(service);
    repo->getRepo(did, QString());
}

void LogManager::clearDb(const QString &did)
{
    m_logAccess.removeDbFile(did);
}
