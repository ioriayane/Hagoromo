#include "columnlistmodel.h"
#include "common.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QUuid>

ColumnListModel::ColumnListModel(QObject *parent) : QAbstractListModel { parent } { }

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

    else if (role == VisibleLikeRole)
        return m_columnList.at(row).type_visibility.like;
    else if (role == VisibleRepostRole)
        return m_columnList.at(row).type_visibility.repost;
    else if (role == VisibleFollowRole)
        return m_columnList.at(row).type_visibility.follow;
    else if (role == VisibleMentionRole)
        return m_columnList.at(row).type_visibility.mention;
    else if (role == VisibleReplyRole)
        return m_columnList.at(row).type_visibility.reply;
    else if (role == VisibleQuoteRole)
        return m_columnList.at(row).type_visibility.quote;

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
        m_columnList[row].value = value.toString();

    else if (role == VisibleLikeRole)
        m_columnList[row].type_visibility.like = value.toBool();
    else if (role == VisibleRepostRole)
        m_columnList[row].type_visibility.repost = value.toBool();
    else if (role == VisibleFollowRole)
        m_columnList[row].type_visibility.follow = value.toBool();
    else if (role == VisibleMentionRole)
        m_columnList[row].type_visibility.mention = value.toBool();
    else if (role == VisibleReplyRole)
        m_columnList[row].type_visibility.reply = value.toBool();
    else if (role == VisibleQuoteRole)
        m_columnList[row].type_visibility.quote = value.toBool();

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
    item.position = row;
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

void ColumnListModel::move(const QString &key, const MoveDirection direction)
{
    // モデルのリストの位置は変更しないで管理indexを移動させる

    int to_position = -1;
    int from_index = -1;
    int to_index = -1;
    for (int i = 0; i < m_columnList.length(); i++) {
        if (m_columnList.at(i).key == key) {
            if (direction == MoveLeft) {
                to_position = m_columnList.at(i).position - 1;
            } else {
                to_position = m_columnList.at(i).position + 1;
            }
            from_index = i;
        }
    }
    for (int i = 0; i < m_columnList.length(); i++) {
        if (m_columnList.at(i).position == to_position) {
            to_index = i;
        }
    }
    if (from_index == -1 || to_index == -1)
        return;

    int temp = m_columnList.at(from_index).position;
    m_columnList[from_index].position = m_columnList.at(to_index).position;
    m_columnList[to_index].position = temp;

    emit dataChanged(index(from_index), index(from_index));
    emit dataChanged(index(to_index), index(to_index));

    save();
}

void ColumnListModel::remove(int row)
{
    if (row < 0 || row >= m_columnList.count())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    int remove_pos = m_columnList.at(row).position;
    m_columnList.removeAt(row);
    // 消したカラムよりも位置が大きいものをひとつ詰める
    for (int i = 0; i < m_columnList.length(); i++) {
        if (m_columnList.at(i).position > remove_pos) {
            m_columnList[i].position--;
        }
    }
    endRemoveRows();

    save();
}

// カラム側からの操作で消すとき
void ColumnListModel::removeByKey(const QString &key)
{
    for (int i = 0; i < m_columnList.count(); i++) {
        if (m_columnList.at(i).key == key) {
            remove(i);
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

// 自分のposition-1が入っているアイテムのインデックスを返す
int ColumnListModel::getPreviousRow(const int row)
{
    if (row < 0 || row >= m_columnList.count())
        return -1;

    int pos = m_columnList.at(row).position - 1;
    for (int i = 0; i < m_columnList.count(); i++) {
        if (m_columnList.at(i).position == pos) {
            return i;
        }
    }
    return -1;
}

QList<int> ColumnListModel::getRowListInOrderOfPosition() const
{
    QList<int> row_list;
    for (int i = 0; i < m_columnList.count(); i++) {
        row_list.append(i);
    }
    for (int i = 0; i < row_list.count() - 1; i++) {
        for (int j = i + 1; j < row_list.count(); j++) {
            if (m_columnList.at(row_list.at(i)).position
                > m_columnList.at(row_list.at(j)).position) {
                row_list.swapItemsAt(i, j);
            }
        }
    }
    return row_list;
}

void ColumnListModel::save() const
{
    QSettings settings;

    QJsonArray column_array;
    for (const ColumnItem &item : m_columnList) {
        QJsonObject column_item;
        column_item["position"] = item.position;
        column_item["key"] = item.key;
        column_item["account_uuid"] = item.account_uuid;
        column_item["component_type"] = static_cast<int>(item.component_type);
        column_item["auto_loading"] = item.auto_loading;
        column_item["interval"] = item.loading_interval;
        column_item["width"] = item.width;
        column_item["name"] = item.name;
        column_item["value"] = item.value;

        QJsonObject notification;
        notification["like"] = item.type_visibility.like;
        notification["repost"] = item.type_visibility.repost;
        notification["follow"] = item.type_visibility.follow;
        notification["mention"] = item.type_visibility.mention;
        notification["reply"] = item.type_visibility.reply;
        notification["quote"] = item.type_visibility.quote;
        column_item["type_visibility"] = notification;

        column_array.append(column_item);
    }

    Common::saveJsonDocument(QJsonDocument(column_array), QStringLiteral("column.json"));
}

void ColumnListModel::load()
{
    if (!m_columnList.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        m_columnList.clear();
        endRemoveRows();
    }

    QJsonDocument doc = Common::loadJsonDocument(QStringLiteral("column.json"));

    if (doc.isArray() && !doc.array().isEmpty()) {
        beginInsertRows(QModelIndex(), 0, doc.array().count() - 1);
        for (int i = 0; i < doc.array().count(); i++) {
            if (doc.array().at(i).isObject()) {
                QJsonObject obj = doc.array().at(i).toObject();
                ColumnItem item;
                item.position = obj.value("position").toInt(-1);
                item.key = obj.value("key").toString();
                item.account_uuid = obj.value("account_uuid").toString();
                item.component_type =
                        static_cast<FeedComponentType>(obj.value("component_type").toInt());
                item.auto_loading = obj.value("auto_loading").toBool(false);
                item.loading_interval = obj.value("interval").toInt(300000);
                item.width = obj.value("width").toInt(400);
                item.name = obj.value("name").toString();
                item.value = obj.value("value").toString();

                item.type_visibility.like =
                        obj.value("type_visibility").toObject().value("like").toBool(true);
                item.type_visibility.repost =
                        obj.value("type_visibility").toObject().value("repost").toBool(true);
                item.type_visibility.follow =
                        obj.value("type_visibility").toObject().value("follow").toBool(true);
                item.type_visibility.mention =
                        obj.value("type_visibility").toObject().value("mention").toBool(true);
                item.type_visibility.reply =
                        obj.value("type_visibility").toObject().value("reply").toBool(true);
                item.type_visibility.quote =
                        obj.value("type_visibility").toObject().value("quote").toBool(true);

                if (item.name.isEmpty()) {
                    // version 0.2.0以前との互換性のため
                    switch (item.component_type) {
                    case FeedComponentType::Timeline:
                        item.name = tr("Following");
                        break;
                    case FeedComponentType::Notification:
                        item.name = tr("Notification");
                        break;
                    case FeedComponentType::SearchPosts:
                        item.name = tr("Search posts");
                        break;
                    case FeedComponentType::SearchProfiles:
                        item.name = tr("Search users");
                        break;
                    default:
                        break;
                    }
                }
                m_columnList.append(item);
            }
        }
        validateIndex();
        endInsertRows();
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

    roles[VisibleLikeRole] = "visibleLike";
    roles[VisibleRepostRole] = "visibleRepost";
    roles[VisibleFollowRole] = "visibleFollow";
    roles[VisibleMentionRole] = "visibleMention";
    roles[VisibleReplyRole] = "visibleReply";
    roles[VisibleQuoteRole] = "visibleQuote";

    return roles;
}

void ColumnListModel::validateIndex()
{
    QList<int> values;
    for (const auto &item : qAsConst(m_columnList)) {
        if (!values.contains(item.position)) {
            values.append(item.position);
        }
    }
    bool valid = true;
    if (values.length() != m_columnList.length()) {
        valid = false;
    } else {
        for (int i = 0; i < values.length() - 1; i++) {
            for (int j = i + 1; j < values.length(); j++) {
                if (values[i] > values[j]) {
                    values.swapItemsAt(i, j);
                }
            }
        }
        for (int i = 0; i < values.length() - 1; i++) {
            if (values[i] + 1 != values[i + 1]) {
                valid = false;
                break;
            }
        }
    }
    if (!valid) {
        for (int i = 0; i < m_columnList.length(); i++) {
            m_columnList[i].position = i;
        }
    }
}
