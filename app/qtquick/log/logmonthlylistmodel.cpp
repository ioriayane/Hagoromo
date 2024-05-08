#include "logmonthlylistmodel.h"
#include "log/logmanager.h"

LogMonthlyListModel::LogMonthlyListModel(QObject *parent) : LogStatisticsListModel { parent } { }

void LogMonthlyListModel::getLatest()
{
    if (running() || did().isEmpty())
        return;
    setRunning(true);

    clear();

    LogManager *manager = new LogManager(this);
    connect(manager, &LogManager::finishedTotals, this,
            [=](const QList<TotalItem> &list, const int max) {
                if (!list.isEmpty()) {
                    setTotalMax(max);

                    beginInsertRows(QModelIndex(), 0, list.count() - 1);
                    m_totalList = list;
                    endInsertRows();
                }
                emit finished();
                setRunning(false);
                manager->deleteLater();
            });
    emit manager->monthlyTotals(did());
}
