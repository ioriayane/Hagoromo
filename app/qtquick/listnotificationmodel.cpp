#include "listnotificationmodel.h"
#include "../atprotocol/appbskynotificationlistnotifications.h"

using AtProtocolInterface::AppBskyNotificationListNotifications;

ListNotificationModel::ListNotificationModel(QObject *parent) : QAbstractListModel { parent } { }

int ListNotificationModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_cidList.count();
}

QVariant ListNotificationModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<ListNotificationModelRoles>(role));
}

QVariant ListNotificationModel::item(int row, ListNotificationModelRoles role) const
{
    if (row < 0 || row >= m_cidList.count())
        return QVariant();

    const auto &current = m_notificationHash.value(m_cidList.at(row));

    if (role == DisplayNameRole)
        return current.author.displayName;

    return QVariant();
}

void ListNotificationModel::setAccount(const QString &service, const QString &did,
                                       const QString &handle, const QString &email,
                                       const QString &accessJwt, const QString &refreshJwt)
{
    m_account.service = service;
    m_account.did = did;
    m_account.handle = handle;
    m_account.email = email;
    m_account.accessJwt = accessJwt;
    m_account.refreshJwt = refreshJwt;
}

void ListNotificationModel::getLatest()
{
    AppBskyNotificationListNotifications *notification = new AppBskyNotificationListNotifications();
    connect(notification, &AppBskyNotificationListNotifications::finished, [=](bool success) {
        //
        if (success) {
            beginInsertRows(QModelIndex(), 0, notification->notificationList()->count() - 1);
            for (const auto &item : *notification->notificationList()) {
                m_cidList.append(item.cid);
                m_notificationHash[item.cid] = item;
            }
            endInsertRows();
        }
        notification->deleteLater();
    });
    notification->setAccount(m_account);
    notification->listNotifications();
}

QHash<int, QByteArray> ListNotificationModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[DisplayNameRole] = "displayName";

    return roles;
}
