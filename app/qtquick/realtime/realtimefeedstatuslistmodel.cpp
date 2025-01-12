#include "realtimefeedstatuslistmodel.h"

using namespace RealtimeFeed;

RealtimeFeedStatusListModel::RealtimeFeedStatusListModel(QObject *parent)
    : QAbstractListModel { parent }
{

    {
        FeedStatusData data;
        data.id = "__status";
        data.name = "Status";
        data.value = "";
        data.unit.clear();
        m_feedStatusData[data.id] = data;
        m_feedStatusIds.append(data.id);
    }
    {
        FeedStatusData data;
        data.id = "__difference";
        data.name = "Difference";
        data.value = "0";
        data.unit = "msec";
        m_feedStatusData[data.id] = data;
        m_feedStatusIds.append(data.id);
    }
    {
        FeedStatusData data;
        data.id = "__total";
        data.name = "Total";
        data.value = "0";
        data.unit = "ope/sec";
        m_feedStatusData[data.id] = data;
        m_feedStatusIds.append(data.id);
    }

    FirehoseReceiver *receiver = FirehoseReceiver::getInstance();
    connect(receiver, &FirehoseReceiver::statusChanged, this,
            &RealtimeFeedStatusListModel::receiverStatusChanged);
    connect(receiver, &FirehoseReceiver::analysisChanged, this,
            &RealtimeFeedStatusListModel::receiverAnalysisChanged);

    receiverStatusChanged(receiver->status());
}

RealtimeFeedStatusListModel::~RealtimeFeedStatusListModel()
{
    FirehoseReceiver *receiver = FirehoseReceiver::getInstance();
    disconnect(receiver, &FirehoseReceiver::analysisChanged, this,
               &RealtimeFeedStatusListModel::receiverAnalysisChanged);
    disconnect(receiver, &FirehoseReceiver::statusChanged, this,
               &RealtimeFeedStatusListModel::receiverStatusChanged);
}

int RealtimeFeedStatusListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_feedStatusIds.count();
}

QVariant RealtimeFeedStatusListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<RealtimeFeedStatusListModelRoles>(role));
}

QVariant RealtimeFeedStatusListModel::item(int row, RealtimeFeedStatusListModelRoles role) const
{
    if (row < 0 || row >= m_feedStatusIds.count())
        return QVariant();

    const auto data = m_feedStatusData.value(m_feedStatusIds.at(row));

    if (role == NameRole)
        return data.name;
    else if (role == ValueRole)
        return data.value;
    else if (role == UnitRole)
        return data.unit;

    return QVariant();
}

void RealtimeFeedStatusListModel::update(int row, RealtimeFeedStatusListModelRoles role,
                                         const QVariant &value)
{
    if (row < 0 || row >= m_feedStatusIds.count())
        return;

    auto &data = m_feedStatusData[m_feedStatusIds.at(row)];
    QVector<int> roles;

    if (role == NameRole) {
        if (data.name != value.toString()) {
            data.name = value.toString();
            roles << NameRole;
        }
    } else if (role == ValueRole) {
        if (data.value != value.toString()) {
            data.value = value.toString();
            roles << ValueRole;
        }
    } else if (role == UnitRole) {
        if (data.unit != value.toString()) {
            data.unit = value.toString();
            roles << UnitRole;
        }
    }

    if (!roles.isEmpty()) {
        emit dataChanged(index(row), index(row), roles);
    }
}

void RealtimeFeedStatusListModel::receiverStatusChanged(
        FirehoseReceiver::FirehoseReceiverStatus newStatus)
{
    const int row = m_feedStatusIds.indexOf("__status");
    if (!m_feedStatusData.contains("__status") || row < 0)
        return;

    m_feedStatusData["__status"].value = "";
    if (newStatus == FirehoseReceiver::FirehoseReceiverStatus::Connected) {
        m_feedStatusData["__status"].value = "Connected";
    } else if (newStatus == FirehoseReceiver::FirehoseReceiverStatus::Disconnected) {
        m_feedStatusData["__status"].value = "Disconnected";
    } else {
        m_feedStatusData["__status"].value = "Error";
    }
    emit dataChanged(index(row), index(row), QVector<int>() << ValueRole);
}

void RealtimeFeedStatusListModel::receiverAnalysisChanged()
{
    FirehoseReceiver *receiver = FirehoseReceiver::getInstance();
    const QHash<QString, QString> nsids_rps = receiver->nsidsReceivePerSecond();

    QHashIterator<QString, QString> i(nsids_rps);
    while (i.hasNext()) {
        i.next();
        const auto row = m_feedStatusIds.indexOf(i.key());
        if (row >= 0) {
            // update
            update(row, ValueRole, i.value());
        } else {
            // add
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            FeedStatusData data;
            data.id = i.key();
            data.name = i.key();
            data.value = i.value();
            data.unit = QStringLiteral("ope/sec");
            m_feedStatusData[data.id] = data;
            m_feedStatusIds.append(data.id);
            endInsertRows();
        }
    }
}

QHash<int, QByteArray> RealtimeFeedStatusListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[NameRole] = "name";
    roles[ValueRole] = "value";
    roles[UnitRole] = "unit";

    return roles;
}
