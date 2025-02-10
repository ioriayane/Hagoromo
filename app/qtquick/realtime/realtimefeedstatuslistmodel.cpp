#include "realtimefeedstatuslistmodel.h"

using namespace RealtimeFeed;

RealtimeFeedStatusListModel::RealtimeFeedStatusListModel(QObject *parent)
    : QAbstractListModel { parent }, m_theme(0)
{
    FirehoseReceiver *receiver = FirehoseReceiver::getInstance();

    appendStatusData(QStringLiteral("__status"), QStringLiteral("Status"), QStringLiteral(""),
                     QString(), QColor());
    appendStatusData(QStringLiteral("__difference"), QStringLiteral("Difference"),
                     QStringLiteral("0"), QStringLiteral("msec"), QColor());
    appendStatusData(QStringLiteral("__bit_per_sec"), QStringLiteral("Receive"),
                     QStringLiteral("0.0"), QStringLiteral("Mbps"), QColor());
    appendStatusData(QStringLiteral("__total"), QStringLiteral("Total"), QStringLiteral("0"),
                     QStringLiteral("ope/sec"), QColor());

    // jetstreamを使うので基本固定
    appendStatusData(QStringLiteral("app.bsky.feed.post"), QStringLiteral("Post"),
                     QStringLiteral("0"), QStringLiteral("ope/sec"), QColor(255, 255, 0));
    appendStatusData(QStringLiteral("app.bsky.feed.repost"), QStringLiteral("Repost"),
                     QStringLiteral("0"), QStringLiteral("ope/sec"), QColor(255, 0, 255));
    appendStatusData(QStringLiteral("app.bsky.feed.like"), QStringLiteral("Like"),
                     QStringLiteral("0"), QStringLiteral("ope/sec"), QColor(0, 255, 255));
    appendStatusData(QStringLiteral("app.bsky.graph.follow"), QStringLiteral("Follow"),
                     QStringLiteral("0"), QStringLiteral("ope/sec"), QColor(255, 0, 0));
    appendStatusData(QStringLiteral("app.bsky.graph.listitem"), QStringLiteral("ListItem"),
                     QStringLiteral("0"), QStringLiteral("ope/sec"), QColor(0, 255, 0));

    updateColorByTheme();

    connect(receiver, &FirehoseReceiver::statusChanged, this,
            &RealtimeFeedStatusListModel::receiverStatusChanged);
    connect(receiver, &FirehoseReceiver::analysisChanged, this,
            &RealtimeFeedStatusListModel::receiverAnalysisChanged);
    connect(receiver, &FirehoseReceiver::serviceEndpointChanged, this,
            &RealtimeFeedStatusListModel::serviceEndpointChangedInFirehose);

    setServiceEndpoint(receiver->serviceEndpoint());
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
    else if (role == UseColorRole)
        return (!data.id.startsWith("_"));
    else if (role == ColorRole)
        return data.color;

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
    switch (newStatus) {
    case FirehoseReceiver::FirehoseReceiverStatus::Connected:
        m_feedStatusData["__status"].value = "Connected";
        break;
    case FirehoseReceiver::FirehoseReceiverStatus::Disconnected:
        m_feedStatusData["__status"].value = "Disconnected";
        break;
    case FirehoseReceiver::FirehoseReceiverStatus::Connecting:
        m_feedStatusData["__status"].value = "Connecting";
        break;
    case FirehoseReceiver::FirehoseReceiverStatus::HostLookup:
        m_feedStatusData["__status"].value = "Looking up host";
        break;
    case FirehoseReceiver::FirehoseReceiverStatus::Bound:
        m_feedStatusData["__status"].value = "Bound";
        break;
    case FirehoseReceiver::FirehoseReceiverStatus::Closing:
        m_feedStatusData["__status"].value = "Closing";
        break;
    default:
        m_feedStatusData["__status"].value = "Error";
        break;
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
            appendStatusData(i.key(), i.key(), i.value(), QStringLiteral("ope/sec"),
                             QColor(128, 128, 128));
            endInsertRows();
        }
    }
}

void RealtimeFeedStatusListModel::serviceEndpointChangedInFirehose(const QString &endpoint)
{
    setServiceEndpoint(endpoint);
}

QHash<int, QByteArray> RealtimeFeedStatusListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[NameRole] = "name";
    roles[ValueRole] = "value";
    roles[UnitRole] = "unit";
    roles[UseColorRole] = "useColor";
    roles[ColorRole] = "color";

    return roles;
}

void RealtimeFeedStatusListModel::appendStatusData(const QString &id, const QString &name,
                                                   const QString &value, const QString &unit,
                                                   const QColor &color)
{
    FeedStatusData data;
    data.id = id;
    data.name = name;
    data.value = value;
    data.unit = unit;
    data.color = color;
    m_feedStatusData[data.id] = data;
    m_feedStatusIds.append(data.id);
}

void RealtimeFeedStatusListModel::updateColorByTheme()
{
    if (theme() == 0) {
        // Light
        m_feedStatusData["app.bsky.feed.post"].color = QColor(0x3F, 0x51, 0xB5); // Indigo
        m_feedStatusData["app.bsky.feed.repost"].color = QColor(0x4C, 0xAF, 0x50); // Green
        m_feedStatusData["app.bsky.feed.like"].color = QColor(0xE9, 0x1E, 0x63); // Pink
        m_feedStatusData["app.bsky.graph.follow"].color = QColor(0x03, 0xA9, 0xF4); // LightBlue
        m_feedStatusData["app.bsky.graph.listitem"].color = QColor(0xFF, 0x98, 0x0); // Orange
    } else {
        // Dark
        m_feedStatusData["app.bsky.feed.post"].color = QColor(0x9F, 0xA8, 0xDA);
        m_feedStatusData["app.bsky.feed.repost"].color = QColor(0xA5, 0xD6, 0xA7);
        m_feedStatusData["app.bsky.feed.like"].color = QColor(0xF4, 0x8F, 0xB1);
        m_feedStatusData["app.bsky.graph.follow"].color = QColor(0x81, 0xD4, 0xFA);
        m_feedStatusData["app.bsky.graph.listitem"].color = QColor(0xFF, 0xCC, 0x80);
    }
    emit dataChanged(index(0), index(rowCount() - 1));
}

int RealtimeFeedStatusListModel::theme() const
{
    return m_theme;
}

void RealtimeFeedStatusListModel::setTheme(int newTheme)
{
    if (m_theme == newTheme)
        return;
    m_theme = newTheme;
    emit themeChanged();

    updateColorByTheme();
}

QString RealtimeFeedStatusListModel::serviceEndpoint() const
{
    return m_serviceEndpoint;
}

void RealtimeFeedStatusListModel::setServiceEndpoint(const QString &newServiceEndpoint)
{
    if (m_serviceEndpoint == newServiceEndpoint)
        return;
    m_serviceEndpoint = newServiceEndpoint;
    emit serviceEndpointChanged();
}
