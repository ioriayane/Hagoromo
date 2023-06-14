#include "columnlistmodel.h"
#include "common.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QUuid>

ColumnListModel::ColumnListModel(QObject *parent) : QAbstractListModel { parent }
{
    load();
}

int ColumnListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_columnList.count();
}

QVariant ColumnListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<ColumnListModelRoles>(role));
}

QVariant ColumnListModel::item(int row, ColumnListModelRoles role) const
{
    if (row < 0 || row >= m_columnList.count())
        return QVariant();

    if (role == KeyRole)
        return m_columnList.at(row).key;
    else if (role == AccountUuidRole)
        return m_columnList.at(row).account_uuid;
    else if (role == ComponentTypeRole)
        return static_cast<int>(m_columnList.at(row).component_type);
    else if (role == AutoLoadingRole)
        return m_columnList.at(row).auto_loading;
    else if (role == LoadingIntervalRole)
        return m_columnList.at(row).loading_interval;
    else if (role == WidthRole)
        return m_columnList.at(row).width;
    else if (role == NameRole)
        return m_columnList.at(row).name;
    else if (role == ValueRole)
        return m_columnList.at(row).value;

    return QVariant();
}

void ColumnListModel::update(int row, ColumnListModelRoles role, const QVariant &value)
{
    if (row < 0 || row >= m_columnList.count())
        return;

    if (role == KeyRole)
        m_columnList[row].key = value.toString();
    else if (role == AccountUuidRole)
        m_columnList[row].account_uuid = value.toString();
    else if (role == ComponentTypeRole)
        m_columnList[row].component_type = static_cast<FeedComponentType>(value.toInt());
    else if (role == AutoLoadingRole)
        m_columnList[row].auto_loading = value.toBool();
    else if (role == LoadingIntervalRole)
        m_columnList[row].loading_interval = value.toInt();
    else if (role == WidthRole)
        m_columnList[row].width = value.toInt();
    else if (role == NameRole)
        m_columnList[row].name = value.toString();
    else if (role == ValueRole)
        m_columnList[row].value = value.toInt();

    emit dataChanged(index(row), index(row));

    save();
}

void ColumnListModel::append(const QString &account_uuid, int component_type, bool auto_loading,
                             int interval, int width, const QString &name, const QString &value)
{
    insert(rowCount(), account_uuid, component_type, auto_loading, interval, width, name, value);
}

void ColumnListModel::insert(int row, const QString &account_uuid, int component_type,
                             bool auto_loading, int interval, int width, const QString &name,
                             const QString &value)
{
    if (row < 0 || row > m_columnList.count())
        return;

    ColumnItem item;
    item.key = QUuid::createUuid().toString(QUuid::WithoutBraces);
    item.account_uuid = account_uuid;
    item.component_type = static_cast<FeedComponentType>(component_type);
    item.auto_loading = auto_loading;
    item.loading_interval = interval;
    item.width = width;
    item.name = name;
    item.value = value;

    beginInsertRows(QModelIndex(), row, row);
    m_columnList.insert(row, item);
    endInsertRows();

    save();
}

void ColumnListModel::remove(int row)
{
    if (row < 0 || row >= m_columnList.count())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_columnList.removeAt(row);
    endRemoveRows();

    save();
}

// カラム側からの操作で消すとき
void ColumnListModel::removeByKey(const QString &key)
{
    for (int i = 0; i < m_columnList.count(); i++) {
        if (m_columnList.at(i).key == key) {
            beginRemoveRows(QModelIndex(), i, i);
            m_columnList.removeAt(i);
            endRemoveRows();

            save();
            break;
        }
    }
}

bool ColumnListModel::containsKey(const QString &key) const
{
    for (const auto &item : m_columnList) {
        if (item.key == key) {
            return true;
        }
    }
    return false;
}

int ColumnListModel::indexOf(const QString &key) const
{
    for (int i = 0; i < m_columnList.count(); i++) {
        if (m_columnList.at(i).key == key) {
            return i;
        }
    }
    return -1;
}

void ColumnListModel::save() const
{
    QSettings settings;

    QJsonArray column_array;
    for (const ColumnItem &item : m_columnList) {
        QJsonObject column_item;
        column_item["key"] = item.key;
        column_item["account_uuid"] = item.account_uuid;
        column_item["component_type"] = static_cast<int>(item.component_type);
        column_item["auto_loading"] = item.auto_loading;
        column_item["interval"] = item.loading_interval;
        column_item["width"] = item.width;
        column_item["name"] = item.name;
        column_item["value"] = item.value;
        column_array.append(column_item);
    }

    Common::saveJsonDocument(QJsonDocument(column_array), QStringLiteral("column.json"));
}

void ColumnListModel::load()
{
    m_columnList.clear();

    QJsonDocument doc = Common::loadJsonDocument(QStringLiteral("column.json"));

    if (doc.isArray()) {
        for (int i = 0; i < doc.array().count(); i++) {
            if (doc.array().at(i).isObject()) {
                ColumnItem item;
                item.key = doc.array().at(i).toObject().value("key").toString();
                item.account_uuid = doc.array().at(i).toObject().value("account_uuid").toString();
                item.component_type = static_cast<FeedComponentType>(
                        doc.array().at(i).toObject().value("component_type").toInt());
                item.auto_loading =
                        doc.array().at(i).toObject().value("auto_loading").toBool(false);
                item.loading_interval =
                        doc.array().at(i).toObject().value("interval").toInt(300000);
                item.width = doc.array().at(i).toObject().value("width").toInt(400);
                item.name = doc.array().at(i).toObject().value("name").toString();
                item.value = doc.array().at(i).toObject().value("value").toString();

                if (item.name.isEmpty()) {
                    // version 0.2.0以前との互換性のため
                    if (item.component_type == FeedComponentType::Timeline) {
                        item.name = tr("Following");
                    } else if (item.component_type == FeedComponentType::Notification) {
                        item.name = tr("Notification");
                    }
                }

                m_columnList.append(item);
            }
        }
    }
}

QHash<int, QByteArray> ColumnListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[KeyRole] = "key";
    roles[AccountUuidRole] = "accountUuid";
    roles[ComponentTypeRole] = "componentType";
    roles[AutoLoadingRole] = "autoLoading";
    roles[LoadingIntervalRole] = "loadingInterval";
    roles[WidthRole] = "width";
    roles[NameRole] = "name";
    roles[ValueRole] = "value";

    return roles;
}
