#include "chatnotificationlistmodel.h"

ChatNotificationListModel::ChatNotificationListModel(QObject *parent)
    : AtpChatAbstractListModel { parent }
{
}

int ChatNotificationListModel::rowCount(const QModelIndex &parent) const
{
    return 0;
}

QVariant ChatNotificationListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<ChatNotificationListModelRoles>(role));
}

QVariant ChatNotificationListModel::item(int row, ChatNotificationListModelRoles role) const
{
    return QVariant();
}

bool ChatNotificationListModel::getLatest()
{
    //
    return true;
}

bool ChatNotificationListModel::getNext()
{
    //
    return true;
}

QHash<int, QByteArray> ChatNotificationListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[IdRole] = "id";
    roles[RevRole] = "rev";

    roles[MemberHandlesRole] = "memberHandles";
    roles[MemberDisplayNamesRole] = "memberDisplayNames";
    roles[MemberAvatarsRole] = "memberAvatars";

    roles[LastMessageIdRole] = "lastMessageId";
    roles[LastMessageRevRole] = "lastMessageRev";
    roles[LastMessageSenderDidRole] = "lastMessageSenderDid";
    roles[LastMessageTextRole] = "lastMessageText";
    roles[LastMessageSentAtRole] = "lastMessageSentAt";

    roles[MutedRole] = "muted";
    roles[UnreadCountRole] = "unreadCount";
    roles[RunningRole] = "running";

    return roles;
}
