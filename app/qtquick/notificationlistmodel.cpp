#include "notificationlistmodel.h"
#include "../atprotocol/appbskynotificationlistnotifications.h"

using AtProtocolInterface::AppBskyNotificationListNotifications;

NotificationListModel::NotificationListModel(AtpAbstractListModel *parent)
    : AtpAbstractListModel { parent }
{
}

int NotificationListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_cidList.count();
}

QVariant NotificationListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<NotificationListModelRoles>(role));
}

QVariant NotificationListModel::item(int row, NotificationListModelRoles role) const
{
    if (row < 0 || row >= m_cidList.count())
        return QVariant();

    const auto &current = m_notificationHash.value(m_cidList.at(row));

    if (role == DisplayNameRole)
        return current.author.displayName;
    else if (role == HandleRole)
        return current.author.handle;
    else if (role == AvatarRole)
        return current.author.avatar;
    else if (role == IndexedAtRole)
        return formatDateTime(current.indexedAt);

    else if (role == ReasonRole) {
        if (current.reason == "like") {
            return NotificationListModelReason::ReasonLike;
        } else if (current.reason == "repost") {
            return NotificationListModelReason::ReasonRepost;
        } else if (current.reason == "follow") {
            return NotificationListModelReason::ReasonFollow;
        } else if (current.reason == "mention") {
            return NotificationListModelReason::ReasonMention;
        } else if (current.reason == "reply") {
            return NotificationListModelReason::ReasonReply;
        } else if (current.reason == "quote") {
            return NotificationListModelReason::ReasonQuote;
        } else {
            return NotificationListModelReason::ReasonUnknown;
        }
    }

    return QVariant();
}

void NotificationListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    AppBskyNotificationListNotifications *notification = new AppBskyNotificationListNotifications();
    connect(notification, &AppBskyNotificationListNotifications::finished, [=](bool success) {
        //
        if (success) {
            QDateTime reference_time;
            if (m_cidList.count() > 0 && m_notificationHash.count() > 0) {
                reference_time = QDateTime::fromString(
                        m_notificationHash[m_cidList.at(0)].indexedAt, Qt::ISODateWithMs);
            } else if (notification->notificationList()->count() > 0) {
                reference_time = QDateTime::fromString(
                        notification->notificationList()->last().indexedAt, Qt::ISODateWithMs);
            } else {
                reference_time = QDateTime::currentDateTimeUtc();
            }
            for (auto item = notification->notificationList()->crbegin();
                 item != notification->notificationList()->crend(); item++) {
                m_notificationHash[item->cid] = *item;

                if (m_cidList.contains(item->cid)) {
                    // リストは更新しないでデータのみ入れ替える
                    // リプライ数とかだけ更新をUIに通知
                    // （取得できた範囲でしか更新できないのだけど・・・）
                    int pos = m_cidList.indexOf(item->cid);
                    emit dataChanged(index(pos), index(pos));
                } else {
                    beginInsertRows(QModelIndex(), 0, 0);
                    m_cidList.insert(0, item->cid);
                    endInsertRows();
                }
            }

            //
            // m_cidList[cid] :
            //   表示リスト（replyとquoteはそのPostのcidを入れる。それ以外は元Postのcidを表示リストに入れて集計表示する（予定））
            // m_list2notificationHash<cid, QList<cid>> :
            //   表示リストのcidに関連している実体のcidのリスト
            // m_notificationHash<cid, Notification> :
            //   apiで取得できるcidをキーにそのまま保存
            // m_postHash<cid, Post> :
            //   Notificationの先にあるPostの実体
            // m_cueGetPost[cid] :
            //   Notificationの先にあるPostを取りに行く待ち行列（たぶんいくつも並列でいけるけど）
            //
            // likeとかの対象ポストの情報は入っていないので、それぞれ取得する必要あり
            // 対象ポスト情報は別途cidをキーにして保存する（2重取得と管理を避ける）
        }
        notification->deleteLater();
        setRunning(false);
    });
    notification->setAccount(account());
    notification->listNotifications();
}

QHash<int, QByteArray> NotificationListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[DisplayNameRole] = "displayName";
    roles[HandleRole] = "handle";
    roles[AvatarRole] = "avatar";
    roles[IndexedAtRole] = "indexedAt";

    roles[ReasonRole] = "reason";

    return roles;
}
