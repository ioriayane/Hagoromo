#include "notificationlistmodel.h"
#include "atprotocol/app/bsky/notification/appbskynotificationlistnotifications.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"

#include <QPointer>
#include <QTimer>

using AtProtocolInterface::AppBskyFeedGetPosts;
using AtProtocolInterface::AppBskyNotificationListNotifications;

NotificationListModel::NotificationListModel(QObject *parent) : AtpAbstractListModel { parent } { }

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

    if (role == CidRole)
        return current.cid;
    else if (role == UriRole)
        return current.uri;
    else if (role == DidRole)
        return current.author.did;
    else if (role == DisplayNameRole)
        return current.author.displayName;
    else if (role == HandleRole)
        return current.author.handle;
    else if (role == AvatarRole)
        return current.author.avatar;
    else if (role == RecordTextRole)
        return copyRecordText(current.record);
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
            AtProtocolType::AppBskyFeedLike::Main like =
                    AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                            AtProtocolType::AppBskyFeedLike::Main>(current.record);
            record_cid = like.subject.cid;
        } else if (current.reason == "repost") {
            AtProtocolType::AppBskyFeedRepost::Main repost =
                    AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                            AtProtocolType::AppBskyFeedRepost::Main>(current.record);
            record_cid = repost.subject.cid;
        } else if (current.reason == "follow") {
        } else if (current.reason == "mention") {
        } else if (current.reason == "reply") {
        } else if (current.reason == "quote") {
            AtProtocolType::AppBskyFeedPost::Main post =
                    AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                            AtProtocolType::AppBskyFeedPost::Main>(current.record);
            switch (post.embed_type) {
            case AtProtocolType::AppBskyFeedPost::MainEmbedType::embed_AppBskyEmbedImages_Main:
                //                post.embed_AppBskyEmbedImages_Main.images;
                // LexiconsTypeUnknown::copyImagesFromPostView(current.post, true)
                break;
            case AtProtocolType::AppBskyFeedPost::MainEmbedType::embed_AppBskyEmbedExternal_Main:
                break;
            case AtProtocolType::AppBskyFeedPost::MainEmbedType::embed_AppBskyEmbedRecord_Main:
                record_cid = post.embed_AppBskyEmbedRecord_Main.record.cid;
                break;
            case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                    embed_AppBskyEmbedRecordWithMedia_Main:
                break;
            default:
                break;
            }
        }
        if (role == RecordCidRole) {
            if (m_postHash.contains(record_cid))
                return m_postHash[record_cid].cid;
            else
                return QString();
        } else if (role == RecordUriRole) {
            if (m_postHash.contains(record_cid))
                return m_postHash[record_cid].uri;
            else
                return QString();
        } else if (role == RecordDisplayNameRole) {
            if (m_postHash.contains(record_cid))
                return m_postHash[record_cid].author.displayName;
            else
                return QString();
        } else if (role == RecordHandleRole) {
            if (m_postHash.contains(record_cid))
                return m_postHash[record_cid].author.handle;
            else
                return QString();
        } else if (role == RecordAvatarRole) {
            if (m_postHash.contains(record_cid))
                return m_postHash[record_cid].author.avatar;
            else
                return QString();
        } else if (role == RecordIndexedAtRole) {
            if (m_postHash.contains(record_cid))
                return formatDateTime(m_postHash[record_cid].indexedAt);
            else
                return QString();
        } else if (role == RecordRecordTextRole) {
            if (m_postHash.contains(record_cid))
                return copyRecordText(m_postHash[record_cid].record);
            else
                return QString();
        } else if (role == RecordImagesRole) {
            if (m_postHash.contains(record_cid))
                return AtProtocolType::LexiconsTypeUnknown::copyImagesFromPostView(
                        m_postHash[record_cid], true);
            else
                return QString();
        } else if (role == RecordImagesFullRole) {
            if (m_postHash.contains(record_cid))
                return AtProtocolType::LexiconsTypeUnknown::copyImagesFromPostView(
                        m_postHash[record_cid], false);
            else
                return QString();
        }
    }

    return QVariant();
}

int NotificationListModel::indexOf(const QString &cid) const
{
    Q_UNUSED(cid)
    return -1;
}

QString NotificationListModel::getRecordText(const QString &cid)
{
    Q_UNUSED(cid)
    return QString();
}

void NotificationListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    QPointer<NotificationListModel> aliving(this);

    AppBskyNotificationListNotifications *notification = new AppBskyNotificationListNotifications();
    connect(notification, &AppBskyNotificationListNotifications::finished, [=](bool success) {
        //
        if (aliving) {
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
                        appendGetPostCue<AtProtocolType::AppBskyFeedLike::Main>(item->record);
                    } else if (item->reason == "repost") {
                        appendGetPostCue<AtProtocolType::AppBskyFeedRepost::Main>(item->record);
                    } else if (item->reason == "quote") {
                        AtProtocolType::AppBskyFeedPost::Main post =
                                AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                                        AtProtocolType::AppBskyFeedPost::Main>(item->record);
                        switch (post.embed_type) {
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedImages_Main:
                            break;
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedExternal_Main:
                            break;
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedRecord_Main:
                            if (!post.embed_AppBskyEmbedRecord_Main.record.cid.isEmpty()
                                && !m_cueGetPost.contains(
                                        post.embed_AppBskyEmbedRecord_Main.record.uri)) {
                                m_cueGetPost.append(post.embed_AppBskyEmbedRecord_Main.record.uri);
                            }
                            break;
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedRecordWithMedia_Main:
                            break;
                        default:
                            break;
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
            setRunning(false);
        }
        notification->deleteLater();
    });
    notification->setAccount(account());
    notification->listNotifications();
}

QHash<int, QByteArray> NotificationListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[CidRole] = "cid";
    roles[UriRole] = "uri";
    roles[DidRole] = "did";
    roles[DisplayNameRole] = "displayName";
    roles[HandleRole] = "handle";
    roles[AvatarRole] = "avatar";
    roles[RecordTextRole] = "recordText";
    roles[IndexedAtRole] = "indexedAt";

    roles[ReasonRole] = "reason";

    roles[RecordCidRole] = "recordCid";
    roles[RecordUriRole] = "recordUri";
    roles[RecordDisplayNameRole] = "recordDisplayName";
    roles[RecordHandleRole] = "recordHandle";
    roles[RecordAvatarRole] = "recordAvatar";
    roles[RecordIndexedAtRole] = "recordIndexedAt";
    roles[RecordRecordTextRole] = "recordRecordText";
    roles[RecordImagesRole] = "recordImages";
    roles[RecordImagesFullRole] = "recordImagesFull";

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

    QPointer<NotificationListModel> aliving(this);

    AppBskyFeedGetPosts *posts = new AppBskyFeedGetPosts();
    connect(posts, &AppBskyFeedGetPosts::finished, [=](bool success) {
        if (aliving) {
            if (success) {
                QStringList new_cid;
                for (const auto &post : *posts->postList()) {
                    //                qDebug() << post.cid << post.author.displayName;
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
                        //                    AtProtocolType::AppBskyFeedLike::Main like =
                        //                            AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                        //                                    AtProtocolType::AppBskyFeedLike::Main>(
                        //                                    m_notificationHash[m_cidList.at(i)].record);
                        //                    if (new_cid.contains(like.subject.cid)) {
                        //                        // データを取得できた
                        //                        emit dataChanged(index(i), index(i));
                        //                    }
                        emitRecordDataChanged<AtProtocolType::AppBskyFeedLike::Main>(
                                i, new_cid, m_notificationHash[m_cidList.at(i)].record);
                    } else if (m_notificationHash[m_cidList.at(i)].reason == "repost") {
                        emitRecordDataChanged<AtProtocolType::AppBskyFeedRepost::Main>(
                                i, new_cid, m_notificationHash[m_cidList.at(i)].record);
                    } else if (m_notificationHash[m_cidList.at(i)].reason == "quote") {
                        AtProtocolType::AppBskyFeedPost::Main post =
                                AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                                        AtProtocolType::AppBskyFeedPost::Main>(
                                        m_notificationHash[m_cidList.at(i)].record);
                        switch (post.embed_type) {
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedImages_Main:
                            break;
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedExternal_Main:
                            break;
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedRecord_Main:
                            if (new_cid.contains(post.embed_AppBskyEmbedRecord_Main.record.cid)) {
                                emit dataChanged(index(i), index(i));
                            }
                            break;
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedRecordWithMedia_Main:
                            break;
                        default:
                            break;
                        }
                    }
                }

                // 残ってたらもう1回
                if (!m_cueGetPost.isEmpty()) {
                    QTimer::singleShot(100, this, &NotificationListModel::getPosts);
                }
            }
        }
        posts->deleteLater();
    });
    posts->setAccount(account());
    posts->getPosts(uris);
}

template<typename T>
void NotificationListModel::appendGetPostCue(const QVariant &record)
{
    T data = AtProtocolType::LexiconsTypeUnknown::fromQVariant<T>(record);
    if (!data.subject.cid.isEmpty() && !m_cueGetPost.contains(data.subject.uri)) {
        m_cueGetPost.append(data.subject.uri);
    }
}

template<typename T>
void NotificationListModel::emitRecordDataChanged(const int i, const QStringList &new_cid,
                                                  const QVariant &record)
{
    T data = AtProtocolType::LexiconsTypeUnknown::fromQVariant<T>(record);
    if (new_cid.contains(data.subject.cid)) {
        // データを取得できた
        emit dataChanged(index(i), index(i));
    }
}
