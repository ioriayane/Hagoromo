#include "stamphistorymodel.h"
#include "imageeditrenderer.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QUrl>

#include <algorithm>

#define STAMP_MAX_HISTORY 20
#define STAMP_MAX_PINNED 5
#define STAMP_MAX_LENGTH 2048
#define STAMP_INDEX_FILE "index.json"

StampHistoryModel::StampHistoryModel(QObject *parent)
    : QAbstractListModel { parent }, m_folder(defaultFolder()), m_lastTick(0)
{
    load();
}

int StampHistoryModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_items.count();
}

QVariant StampHistoryModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<StampHistoryModelRoles>(role));
}

QVariant StampHistoryModel::item(int row, StampHistoryModelRoles role) const
{
    if (row < 0 || row >= m_items.count())
        return QVariant();
    const StampHistoryItem &item = m_items.at(row);
    if (role == IdRole)
        return item.id;
    else if (role == SourceRole)
        return source(item.id);
    else if (role == PinnedRole)
        return item.pinned;
    else if (role == LastUsedRole)
        return item.lastUsed;
    return QVariant();
}

void StampHistoryModel::load()
{
    beginResetModel();
    m_items.clear();
    QFile file(QDir(m_folder).filePath(STAMP_INDEX_FILE));
    if (file.open(QIODevice::ReadOnly)) {
        const QJsonArray items =
                QJsonDocument::fromJson(file.readAll()).object().value("items").toArray();
        for (const auto &value : items) {
            const QJsonObject obj = value.toObject();
            StampHistoryItem item;
            item.id = obj.value("id").toString();
            item.pinned = obj.value("pinned").toBool();
            item.lastUsed = static_cast<qint64>(obj.value("lastUsed").toDouble());
            item.pinnedAt = static_cast<qint64>(obj.value("pinnedAt").toDouble());
            // 画像が無いものは除外する
            if (item.id.isEmpty() || indexOf(item.id) >= 0 || !QFile::exists(filePath(item.id)))
                continue;
            m_items.append(item);
            m_lastTick = qMax(m_lastTick, qMax(item.lastUsed, item.pinnedAt));
        }
    }
    sortItems();
    endResetModel();
    emit countChanged();
}

QString StampHistoryModel::add(const QString &url)
{
    const QUrl image_url(url);
    const QString path = image_url.isLocalFile() ? image_url.toLocalFile() : url;
    QImage image = ImageEditRenderer::load(path);
    if (image.isNull())
        return QString();
    if (qMax(image.width(), image.height()) > STAMP_MAX_LENGTH) {
        image = image.scaled(STAMP_MAX_LENGTH, STAMP_MAX_LENGTH, Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);
    }

    QByteArray png;
    {
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        if (!image.save(&buffer, "PNG"))
            return QString();
    }
    const QString id =
            QString::fromLatin1(QCryptographicHash::hash(png, QCryptographicHash::Sha1).toHex());

    QDir().mkpath(m_folder);
    const QString file_path = filePath(id);
    if (!QFile::exists(file_path)) {
        QFile file(file_path);
        if (!file.open(QIODevice::WriteOnly) || file.write(png) != png.size()) {
            file.close();
            QFile::remove(file_path);
            return QString();
        }
    }

    const int row = indexOf(id);
    if (row >= 0) {
        m_items[row].lastUsed = tick();
    } else {
        StampHistoryItem item;
        item.id = id;
        item.lastUsed = tick();
        m_items.append(item);
    }
    update();
    return id;
}

void StampHistoryModel::touch(const QString &id)
{
    const int row = indexOf(id);
    if (row < 0)
        return;
    m_items[row].lastUsed = tick();
    update();
}

bool StampHistoryModel::setPinned(const QString &id, bool pinned)
{
    const int row = indexOf(id);
    if (row < 0)
        return false;
    if (m_items.at(row).pinned == pinned)
        return true;
    if (pinned && pinnedCount() >= STAMP_MAX_PINNED)
        return false;
    m_items[row].pinned = pinned;
    if (pinned) {
        m_items[row].pinnedAt = tick();
    } else {
        // ピン止めを外した直後に履歴から消えないように使用したことにする
        m_items[row].pinnedAt = 0;
        m_items[row].lastUsed = tick();
    }
    update();
    return true;
}

void StampHistoryModel::remove(const QString &id)
{
    const int row = indexOf(id);
    if (row < 0)
        return;
    QFile::remove(filePath(id));
    m_items.removeAt(row);
    update();
}

QString StampHistoryModel::source(const QString &id) const
{
    if (id.isEmpty())
        return QString();
    return QUrl::fromLocalFile(filePath(id)).toString();
}

int StampHistoryModel::count() const
{
    return m_items.count();
}

int StampHistoryModel::pinnedCount() const
{
    int count = 0;
    for (const auto &item : m_items) {
        if (item.pinned)
            count++;
    }
    return count;
}

int StampHistoryModel::maxHistory() const
{
    return STAMP_MAX_HISTORY;
}

int StampHistoryModel::maxPinned() const
{
    return STAMP_MAX_PINNED;
}

QString StampHistoryModel::folder() const
{
    return m_folder;
}

void StampHistoryModel::setFolder(const QString &newFolder)
{
    if (m_folder == newFolder)
        return;
    m_folder = newFolder;
    emit folderChanged();
    load();
}

QString StampHistoryModel::defaultFolder()
{
    return QString("%1/stamps")
            .arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
}

QHash<int, QByteArray> StampHistoryModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "stampId";
    roles[SourceRole] = "source";
    roles[PinnedRole] = "pinned";
    roles[LastUsedRole] = "lastUsed";
    return roles;
}

int StampHistoryModel::indexOf(const QString &id) const
{
    for (int i = 0; i < m_items.count(); i++) {
        if (m_items.at(i).id == id)
            return i;
    }
    return -1;
}

QString StampHistoryModel::filePath(const QString &id) const
{
    return QDir(m_folder).filePath(QString("%1.png").arg(id));
}

qint64 StampHistoryModel::tick()
{
    // 同じミリ秒でも順番が分かるように単調増加させる
    m_lastTick = qMax(QDateTime::currentMSecsSinceEpoch(), m_lastTick + 1);
    return m_lastTick;
}

void StampHistoryModel::enforceLimit()
{
    sortItems();
    int unpinned = 0;
    for (int i = 0; i < m_items.count(); i++) {
        if (m_items.at(i).pinned)
            continue;
        unpinned++;
        if (unpinned > STAMP_MAX_HISTORY) {
            // 古いものから削除する（並びは新しい順）
            QFile::remove(filePath(m_items.at(i).id));
            m_items.removeAt(i);
            i--;
        }
    }
}

void StampHistoryModel::sortItems()
{
    // ピン止め（ピン止めした順）-> 履歴（新しい順）
    std::stable_sort(m_items.begin(), m_items.end(),
                     [](const StampHistoryItem &a, const StampHistoryItem &b) {
                         if (a.pinned != b.pinned)
                             return a.pinned;
                         if (a.pinned)
                             return a.pinnedAt < b.pinnedAt;
                         return a.lastUsed > b.lastUsed;
                     });
}

void StampHistoryModel::save() const
{
    QDir().mkpath(m_folder);
    QJsonArray items;
    for (const auto &item : m_items) {
        QJsonObject obj;
        obj.insert("id", item.id);
        obj.insert("pinned", item.pinned);
        obj.insert("lastUsed", static_cast<double>(item.lastUsed));
        obj.insert("pinnedAt", static_cast<double>(item.pinnedAt));
        items.append(obj);
    }
    QJsonObject root;
    root.insert("version", 1);
    root.insert("items", items);
    QFile file(QDir(m_folder).filePath(STAMP_INDEX_FILE));
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    }
}

void StampHistoryModel::update()
{
    beginResetModel();
    enforceLimit();
    endResetModel();
    save();
    emit countChanged();
}
