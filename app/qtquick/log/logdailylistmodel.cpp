#include "logdailylistmodel.h"
#include "log/logmanager.h"

LogDailyListModel::LogDailyListModel(QObject *parent) : LogStatisticsListModel { parent } { }

void LogDailyListModel::getLatest()
{
    if (running() || did().isEmpty())
        return;
    setRunning(true);

    clear();

    LogManager *manager = new LogManager(this);
    connect(manager, &LogManager::finishedTotals, this, [=](const QList<TotalItem> &list) {
        if (!list.isEmpty()) {
            beginInsertRows(QModelIndex(), 0, list.count() - 1);
            m_totalList = list;
            endInsertRows();
        }
        emit finished();
        setRunning(false);
        manager->deleteLater();
    });
    emit manager->dailyTotals(did());
}
