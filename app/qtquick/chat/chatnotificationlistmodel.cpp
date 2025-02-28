#include "chatnotificationlistmodel.h"

ChatNotificationListModel::ChatNotificationListModel(QObject *parent)
    : AtpChatAbstractListModel { parent }
{
}

int ChatNotificationListModel::rowCount(const QModelIndex &parent) const
{
    return m_chatNotificationData.count();
}

QVariant ChatNotificationListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<ChatNotificationListModelRoles>(role));
}

QVariant ChatNotificationListModel::item(int row, ChatNotificationListModelRoles role) const
{
    if (row < 0 || row >= m_chatNotificationData.count())
        return QVariant();

    const auto &current = m_chatNotificationData.at(row);

    switch (role) {
    case AccountUuidRole:
        return current.account_uuid;
    case AvatarRole:
        return current.avatar;
    case UnreadCountRole:
        return current.unread_count;
    case VisibleRole:
        return current.visible;
    default:
        break;
    }

    return QVariant();
}

bool ChatNotificationListModel::getLatest()
{
    //
    return true;
}

bool ChatNotificationListModel::getNext()
{
    return true;
}

void ChatNotificationListModel::start()
{
    {
        ChatNotificationData data;
        data.account_uuid = "4d8578fd-ce13-4ee1-90eb-c50a9248777a";
        data.avatar = "https://cdn.bsky.app/img/avatar/plain/did:plc:ipj5qejfoqu6eukvt72uhyit/"
                      "bafkreifjldy2fbgjfli7dson343u2bepzwypt7vlffb45ipsll6bjklphy@jpeg";
        data.unread_count = 3;
        data.visible = true;
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_chatNotificationData.append(data);
        endInsertRows();
    }
    {
        ChatNotificationData data;
        data.account_uuid = "4d8578fd-ce13-4ee1-90eb-c50a9248777a";
        data.avatar = "https://cdn.bsky.app/img/avatar/plain/did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                      "bafkreiaeoiy6fqjypbhbcrb3jdlnjtpnwri5wa6jrvbwxtbtey6synwxr4@jpeg";
        data.unread_count = 3;
        data.visible = true;
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_chatNotificationData.append(data);
        endInsertRows();
    }
    {
        ChatNotificationData data;
        data.account_uuid = "4d8578fd-ce13-4ee1-90eb-c50a9248777a";
        data.avatar = "https://cdn.bsky.app/img/avatar/plain/did:plc:73l5atmh7p3fn3xigbp6ao5x/"
                      "bafkreif42yycinokltjrfizxgvyyw4z63a264jc4ws723pauxtc7vqo7la@jpeg";
        data.unread_count = 3;
        data.visible = true;
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_chatNotificationData.append(data);
        endInsertRows();
    }
}

void ChatNotificationListModel::hideItem(int row)
{
    if (row < 0 || row >= m_chatNotificationData.count())
        return;

    m_chatNotificationData[row].visible = false;

    emit dataChanged(index(row), index(row));
}

QHash<int, QByteArray> ChatNotificationListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[AccountUuidRole] = "accountUuid";
    roles[AvatarRole] = "avatar";
    roles[UnreadCountRole] = "unreadCount";
    roles[VisibleRole] = "visible";

    return roles;
}
