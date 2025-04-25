#include "emojilistmodel.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>

EmojiListModel::EmojiListModel(QObject *parent) : QAbstractListModel { parent }, m_columnCount(7)
{
    load();
}

int EmojiListModel::rowCount(const QModelIndex &row) const
{
    return m_emojiRowDataList.count();
}

QVariant EmojiListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), index.column(), static_cast<EmojiListModelRoles>(role));
}

QVariant EmojiListModel::item(int row, int column, EmojiListModelRoles role) const
{
    if (row < 0 || row >= rowCount())
        return QVariant();

    switch (role) {
    case GroupNameRole:
        return m_emojiRowDataList.at(row).group_name;
    case EmojisRole: {
        QStringList emojis;
        for (const auto &emoji_data : m_emojiRowDataList.at(row).emojis) {
            emojis.append(emoji_data.emoji);
        }
        return emojis;
    }
    }

    return QVariant();
}

QHash<int, QByteArray> EmojiListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[GroupNameRole] = "groupName";
    roles[EmojisRole] = "emojis";

    return roles;
}

QString EmojiListModel::getGroupName(int index) const
{
    if (index < 0 || index >= m_groupList.count())
        return QString();

    return m_groupList.at(index);
}

int EmojiListModel::getGroupTopRow(int group_index) const
{
    if (m_emojiRowDataList.isEmpty())
        return 0;

    QString prev;
    int index = 0;
    for (int row = 0; row < m_emojiRowDataList.count(); row++) {
        const auto &row_data = m_emojiRowDataList.at(row);
        if (prev != row_data.group_name) {
            if (index == group_index) {
                return row;
            }
            index++;
        }
        prev = row_data.group_name;
    }
    return 0;
}

int EmojiListModel::getGroupIndex(const QString &group_name) const
{
    for (int i = 0; i < m_groupList.count(); i++) {
        if (m_groupList.at(i) == group_name) {
            return i;
        }
    }
    return -1;
}

void EmojiListModel::setFrequentlyUsed(const QString &emoji)
{
    int row = m_emojiRowDataIndex.value(emoji, -1);
    if (row < 0 || m_emojiRowDataList.isEmpty())
        return;

    while (m_emojiRowDataList.first().emojis.count() >= columnCount()) {
        m_emojiRowDataList.first().emojis.pop_back();
    }
    for (int i = 0; i < m_emojiRowDataList.first().emojis.length(); i++) {
        if (m_emojiRowDataList.first().emojis.at(i).emoji == emoji) {
            m_emojiRowDataList.first().emojis.remove(i);
            break;
        }
    }
    for (const auto &emoji_data : m_emojiRowDataList.at(row).emojis) {
        if (emoji_data.emoji == emoji) {
            m_emojiRowDataList.first().emojis.push_front(emoji_data);
            break;
        }
    }
    emit dataChanged(index(0), index(0));

    saveFrequentlyUsed();
}

void EmojiListModel::load()
{
    m_groupList.clear();
    m_emojiRowDataIndex.clear();
    if (!m_emojiRowDataList.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        m_emojiRowDataList.clear();
        endRemoveRows();
    }

    QFile file(":/Hagoromo/qtquick/controls/emoji.json");
    if (!file.open(QFile::ReadOnly))
        return;
    QByteArray ba = file.readAll();

    QJsonDocument doc = QJsonDocument::fromJson(ba);
    file.close();
    if (!doc.isArray())
        return;

    int item_count = 0;
    EmojiRowData emoji_row_data;
    EmojiData emoji_data;

    {
        QString name = QStringLiteral("Frequently used");
        m_groupList.append(name);
        emoji_row_data.group_name = name;
        emoji_row_data.emojis.clear();
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_emojiRowDataList.append(emoji_row_data);
        endInsertRows();
    }
    for (const auto &group : doc.array()) {
        QString name = group.toObject().value("name").toString();
        if (name.isEmpty())
            continue;
        m_groupList.append(name);
        emoji_row_data.group_name = name;
        emoji_row_data.emojis.clear();
        item_count = 0;
        for (const auto &emoji : group.toObject().value("emojis").toArray()) {
            emoji_data.emoji = emoji.toObject().value("emoji").toString();
            emoji_data.description = emoji.toObject().value("description").toString();

            if (emoji_data.emoji.isEmpty())
                continue;
            emoji_row_data.emojis.append(emoji_data);

            m_emojiRowDataIndex[emoji_data.emoji] = rowCount();

            if (item_count >= (m_columnCount - 1)) {
                beginInsertRows(QModelIndex(), rowCount(), rowCount());
                m_emojiRowDataList.append(emoji_row_data);
                endInsertRows();
                emoji_row_data.emojis.clear();
                item_count = 0;
            } else {
                item_count++;
            }
        }
        if (!emoji_row_data.emojis.isEmpty()) {
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            m_emojiRowDataList.append(emoji_row_data);
            endInsertRows();
        }
    }

    loadFrequentlyUsed();
}

void EmojiListModel::loadFrequentlyUsed()
{
    if (m_emojiRowDataList.isEmpty())
        return;

    QSettings settings;
    QString emojis = QString::fromUtf8(
            QByteArray::fromBase64(settings.value("frequentyUsedEmojis").toByteArray()));
    for (const auto &emoji : emojis.split("\n")) {
        int row = m_emojiRowDataIndex.value(emoji, -1);
        qDebug() << "emoji" << emoji << row;
        if (row >= 0) {
            for (const auto &emoji_data : m_emojiRowDataList.at(row).emojis) {
                if (emoji_data.emoji == emoji) {
                    m_emojiRowDataList.first().emojis.push_back(emoji_data);
                    break;
                }
            }
        }
    }
    emit dataChanged(index(0), index(0));
}

void EmojiListModel::saveFrequentlyUsed() const
{
    if (m_emojiRowDataList.isEmpty())
        return;

    QSettings settings;

    QStringList emojis;
    for (const auto &emoji_data : m_emojiRowDataList.first().emojis) {
        emojis.append(emoji_data.emoji);
    }
    settings.setValue("frequentyUsedEmojis", emojis.join("\n").toUtf8().toBase64());
}

int EmojiListModel::columnCount() const
{
    return m_columnCount;
}

void EmojiListModel::setColumnCount(int newColumnCount)
{
    if (m_columnCount == newColumnCount)
        return;
    m_columnCount = newColumnCount;
    emit columnCountChanged();

    load();
}
