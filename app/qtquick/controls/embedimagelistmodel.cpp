#include "embedimagelistmodel.h"

#include <QTimer>
#include <QDebug>
#include <QFileInfo>
#include <QUrl>

EmbedImageListModel::EmbedImageListModel(QObject *parent)
    : QAbstractListModel { parent }, m_count(0), m_running(false)
{
}

int EmbedImageListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_embedImageList.count();
}

QVariant EmbedImageListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<EmbedImageListModelRoles>(role));
}

QVariant EmbedImageListModel::item(int row, EmbedImageListModelRoles role) const
{
    if (row < 0 || row >= m_embedImageList.count())
        return QVariant();

    if (role == UriRole)
        return m_embedImageList.at(row).uri;
    else if (role == AltRole)
        return m_embedImageList.at(row).alt;
    else if (role == NumberRole) {
        if (rowCount() > 4) {
            return QString("%1/%2").arg(row + 1).arg(rowCount());
        } else {
            return QString();
        }
    }

    return QVariant();
}

void EmbedImageListModel::update(int row, EmbedImageListModelRoles role, const QVariant &value)
{
    if (row < 0 || row >= m_embedImageList.count())
        return;

    if (role == UriRole)
        m_embedImageList[row].uri = value.toString();
    else if (role == AltRole)
        m_embedImageList[row].alt = value.toString();

    emit dataChanged(index(row), index(row));
}

void EmbedImageListModel::clear()
{
    if (m_embedImageList.isEmpty())
        return;

    beginRemoveRows(QModelIndex(), 0, m_embedImageList.count() - 1);
    m_embedImageList.clear();
    endRemoveRows();

    setCount(rowCount());
}

void EmbedImageListModel::remove(const int row)
{
    if (row < 0 || row >= m_embedImageList.count())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_embedImageList.removeAt(row);
    endRemoveRows();

    setCount(rowCount());

    if (count() > 0) {
        emit dataChanged(index(0), index(count() - 1));
    }
}

bool EmbedImageListModel::append(const QStringList &uris)
{
    if (uris.isEmpty() || running())
        return false;

    setRunning(true);
    m_uriCue.clear();

    QStringList exts;
    exts << "jpg"
         << "jpeg"
         << "png"
         << "gif";
    for (const auto &uri : uris) {
        QFileInfo info(QUrl(uri).toLocalFile());
        if (exts.contains(info.suffix().toLower())) {
            m_uriCue.append(uri);
        }
    }

    QTimer::singleShot(10, this, &EmbedImageListModel::appendItem);

    return !m_uriCue.isEmpty();
}

void EmbedImageListModel::updateAlt(const int row, const QString &alt)
{
    update(row, EmbedImageListModelRoles::AltRole, alt);
}

QStringList EmbedImageListModel::uris() const
{
    QStringList uris;
    for (const auto &item : m_embedImageList) {
        uris.append(item.uri);
    }
    return uris;
}

QStringList EmbedImageListModel::alts() const
{
    QStringList uris;
    for (const auto &item : m_embedImageList) {
        uris.append(item.alt);
    }
    return uris;
}

int EmbedImageListModel::count() const
{
    return m_count;
}

void EmbedImageListModel::setCount(int newCount)
{
    if (m_count == newCount)
        return;
    m_count = newCount;
    emit countChanged();
}

QHash<int, QByteArray> EmbedImageListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[UriRole] = "uri";
    roles[AltRole] = "alt";
    roles[NumberRole] = "number";

    return roles;
}

void EmbedImageListModel::appendItem()
{
    if (m_uriCue.isEmpty()) {
        setRunning(false);
        return;
    }

    EmbedImageItem item;
    item.uri = m_uriCue.first();
    item.alt.clear();

    m_uriCue.pop_front();

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_embedImageList.append(item);
    endInsertRows();

    setCount(rowCount());

    emit dataChanged(index(0), index(count() - 2));

    QTimer::singleShot(10, this, &EmbedImageListModel::appendItem);
}

bool EmbedImageListModel::running() const
{
    return m_running;
}

void EmbedImageListModel::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}
