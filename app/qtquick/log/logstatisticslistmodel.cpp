#include "logstatisticslistmodel.h"
#include "log/logmanager.h"

LogStatisticsListModel::LogStatisticsListModel(QObject *parent)
    : QAbstractListModel { parent }, m_running(false), m_totalMax(0)
{
}

int LogStatisticsListModel::rowCount(const QModelIndex &parent) const
{
    return m_totalList.count();
}

QVariant LogStatisticsListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<LogStatisticsListModelRoles>(role));
}

QVariant LogStatisticsListModel::item(int row, LogStatisticsListModelRoles role) const
{
    if (row < 0 || row >= m_totalList.count())
        return QVariant();

    if (role == GroupRole)
        return m_totalList.at(row).group;
    else if (role == NameRole)
        return m_totalList.at(row).name;
    else if (role == CountRole)
        return m_totalList.at(row).count;
    else if (role == PercentRole) {
        if (totalMax() == 0) {
            return 0.0;
        } else {
            return static_cast<qreal>(m_totalList.at(row).count) / static_cast<qreal>(totalMax());
        }
    }

    return QVariant();
}

void LogStatisticsListModel::getLatest()
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
    emit manager->statistics(did());
}

void LogStatisticsListModel::clear()
{
    if (m_totalList.isEmpty())
        return;

    beginRemoveRows(QModelIndex(), 0, m_totalList.count() - 1);
    m_totalList.clear();
    endRemoveRows();

    setTotalMax(0);
}

QHash<int, QByteArray> LogStatisticsListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[GroupRole] = "group";
    roles[NameRole] = "name";
    roles[CountRole] = "count";
    roles[PercentRole] = "percent";

    return roles;
}

bool LogStatisticsListModel::running() const
{
    return m_running;
}

void LogStatisticsListModel::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

QString LogStatisticsListModel::did() const
{
    return m_did;
}

void LogStatisticsListModel::setDid(const QString &newDid)
{
    if (m_did == newDid)
        return;
    m_did = newDid;
    emit didChanged();
}

int LogStatisticsListModel::totalMax() const
{
    return m_totalMax;
}

void LogStatisticsListModel::setTotalMax(int newTotalMax)
{
    if (m_totalMax == newTotalMax)
        return;
    m_totalMax = newTotalMax;
    emit totalMaxChanged();
}
