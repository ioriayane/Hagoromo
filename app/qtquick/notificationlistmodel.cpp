#include "notificationlistmodel.h"
#include "../atprotocol/appbskynotificationlistnotifications.h"
#include "../atprotocol/lexicons_func_unknown.h"
#include "../atprotocol/appbskyfeedgetposts.h"

#include <QTimer>

using AtProtocolInterface::AppBskyFeedGetPosts;
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
    } else {
        QString record_cid;
        if (current.reason == "like") {
            AtProtocolType::AppBskyFeedLike::Record like =
                    AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                            AtProtocolType::AppBskyFeedLike::Record>(current.record);
            record_cid = like.subject.cid;
        } else if (current.reason == "repost") {
        } else if (current.reason == "follow") {
        } else if (current.reason == "mention") {
        } else if (current.reason == "reply") {
        } else if (current.reason == "quote") {
        }
        if (!record_cid.isEmpty()) {
            if (m_postHash.contains(record_cid)) {
                if (role == RecordDisplayNameRole)
                    return m_postHash[record_cid].author.displayName;
                else if (role == RecordHandleRole)
                    return m_postHash[record_cid].author.handle;
                else if (role == RecordAvatarRole)
                    return m_postHash[record_cid].author.avatar;
                else if (role == RecordIndexedAtRole)
                    return formatDateTime(m_postHash[record_cid].indexedAt);
                else if (role == RecordRecordTextRole)
                    return AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                                   AtProtocolType::AppBskyFeedPost::Record>(
                                   m_postHash[record_cid].record)
                            .text;
            } else {
                if (role == RecordDisplayNameRole)
                    return QString();
                else if (role == RecordHandleRole)
                    return QString();
                else if (role == RecordAvatarRole)
                    return QString();
                else if (role == RecordIndexedAtRole)
                    return QString();
                else if (role == RecordRecordTextRole)
                    return QString();
            }
        } else {
            if (role == RecordDisplayNameRole)
                return QString();
            else if (role == RecordHandleRole)
                return QString();
            else if (role == RecordAvatarRole)
                return QString();
            else if (role == RecordIndexedAtRole)
                return QString();
            else if (role == RecordRecordTextRole)
                return QString();
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

                if (item->reason == "like") {
                    AtProtocolType::AppBskyFeedLike::Record like =
                            AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                                    AtProtocolType::AppBskyFeedLike::Record>(item->record);
                    if (!like.subject.cid.isEmpty() && !m_cueGetPost.contains(like.subject.uri)) {
                        m_cueGetPost.append(like.subject.uri);
                    }
                }
            }

            if (!m_cueGetPost.isEmpty()) {
                QTimer::singleShot(100, this, &NotificationListModel::getPosts);
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

            // m_post2notificationHash<cid, cid>
            //   Post側(m_postHash)からNotification側(m_notificationHash)の参照
            //
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

    roles[RecordDisplayNameRole] = "recordDisplayName";
    roles[RecordHandleRole] = "recordHandle";
    roles[RecordAvatarRole] = "recordAvatar";
    roles[RecordIndexedAtRole] = "recordIndexedAt";
    roles[RecordRecordTextRole] = "recordRecordText";

    return roles;
}

void NotificationListModel::getPosts()
{
    if (m_cueGetPost.isEmpty())
        return;

    // getPostsは最大25個までいっきに取得できる
    QStringList uris;
    for (int i = 0; i < 25; i++) {
        if (m_cueGetPost.isEmpty())
            break;
        uris.append(m_cueGetPost.first());
        m_cueGetPost.removeFirst();
    }

    AppBskyFeedGetPosts *posts = new AppBskyFeedGetPosts();
    connect(posts, &AppBskyFeedGetPosts::finished, [=](bool success) {
        if (success) {
            QStringList new_cid;
            for (const auto &post : *posts->postList()) {
                qDebug() << post.cid << post.author.displayName;
                //                AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                //                        AtProtocolType::AppBskyFeedPost::Record>(post.record)
                //                        .text;
                m_postHash[post.cid] = post;
                new_cid.append(post.cid);
            }

            for (int i = 0; i < m_cidList.count(); i++) {
                if (!m_notificationHash.contains(m_cidList.at(i)))
                    continue;

                if (m_notificationHash[m_cidList.at(i)].reason == "like") {
                    AtProtocolType::AppBskyFeedLike::Record like =
                            AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                                    AtProtocolType::AppBskyFeedLike::Record>(
                                    m_notificationHash[m_cidList.at(i)].record);
                    if (new_cid.contains(like.subject.cid)) {
                        // データを取得できた
                        emit dataChanged(index(i), index(i));
                    }
                }
            }

            // 残ってたらもう1回
            if (!m_cueGetPost.isEmpty()) {
                QTimer::singleShot(100, this, &NotificationListModel::getPosts);
            }
        }
        posts->deleteLater();
    });
    posts->setAccount(account());
    posts->getPosts(uris);
}
