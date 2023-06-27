#include "notificationlistmodel.h"
#include "atprotocol/app/bsky/notification/appbskynotificationlistnotifications.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"
#include "recordoperator.h"

#include <QPointer>
#include <QTimer>

using AtProtocolInterface::AppBskyFeedGetPosts;
using AtProtocolInterface::AppBskyNotificationListNotifications;

NotificationListModel::NotificationListModel(QObject *parent)
    : AtpAbstractListModel { parent },
      m_enabledLike(true),
      m_enabledRepost(true),
      m_enabledFollow(true),
      m_enabledMention(true),
      m_enabledReply(true),
      m_enabledQuote(true)
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

    if (role == VisibleRole)
        return enableReason(current.reason);

    else if (role == CidRole)
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
    else if (role == RecordTextPlainRole)
        return AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                       AtProtocolType::AppBskyFeedPost::Main>(current.record)
                .text;

    else if (role == IndexedAtRole)
        return formatDateTime(current.indexedAt);

    //----------------------------------------
    else if (role == ReplyCountRole) {
        if (m_postHash.contains(current.cid))
            return m_postHash[current.cid].replyCount;
        else
            return 0;
    } else if (role == RepostCountRole) {
        if (m_postHash.contains(current.cid))
            return m_postHash[current.cid].repostCount;
        else
            return 0;
    } else if (role == LikeCountRole) {
        if (m_postHash.contains(current.cid))
            return m_postHash[current.cid].likeCount;
        else
            return 0;
    } else if (role == IsRepostedRole) {
        if (m_postHash.contains(current.cid))
            return m_postHash[current.cid].viewer.repost.contains(account().did);
        else
            return QString();
    } else if (role == IsLikedRole) {
        if (m_postHash.contains(current.cid))
            return m_postHash[current.cid].viewer.like.contains(account().did);
        else
            return QString();
    } else if (role == RepostedUriRole) {
        if (m_postHash.contains(current.cid))
            return m_postHash[current.cid].viewer.repost;
        else
            return QString();
    } else if (role == LikedUriRole) {
        if (m_postHash.contains(current.cid))
            return m_postHash[current.cid].viewer.like;
        else
            return QString();

        //----------------------------------------
    } else if (role == ReasonRole) {
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

        //----------------------------------------
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
        } else if (role == RecordIsRepostedRole) {
            if (m_postHash.contains(record_cid))
                return m_postHash[record_cid].viewer.repost.contains(account().did);
            else
                return false;
        } else if (role == RecordIsLikedRole) {
            if (m_postHash.contains(record_cid))
                return m_postHash[record_cid].viewer.like.contains(account().did);
            else
                return false;
        }
    }

    return QVariant();
}

void NotificationListModel::update(int row, NotificationListModelRoles role, const QVariant &value)
{
    if (row < 0 || row >= m_cidList.count())
        return;

    // 外から更新しない
    // like/repostはユーザー操作を即時反映するため例外

    const auto &current = m_notificationHash.value(m_cidList.at(row));

    if (role == RepostedUriRole) {
        if (m_postHash.contains(current.cid)) {
            qDebug() << "update REPOST" << value.toString();
            m_postHash[current.cid].viewer.repost = value.toString();
            if (m_postHash[current.cid].viewer.repost.isEmpty())
                m_postHash[current.cid].repostCount--;
            else
                m_postHash[current.cid].repostCount++;
            emit dataChanged(index(row), index(row));
        }
    } else if (role == LikedUriRole) {
        if (m_postHash.contains(current.cid)) {
            qDebug() << "update LIKE" << value.toString();
            m_postHash[current.cid].viewer.like = value.toString();
            if (m_postHash[current.cid].viewer.like.isEmpty())
                m_postHash[current.cid].likeCount--;
            else
                m_postHash[current.cid].likeCount++;
            emit dataChanged(index(row), index(row));
        }
    }

    return;
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
        if (aliving) {
            if (success) {
                QDateTime reference_time = QDateTime::currentDateTimeUtc();

                for (auto item = notification->notificationList()->crbegin();
                     item != notification->notificationList()->crend(); item++) {
                    m_notificationHash[item->cid] = *item;

                    PostCueItem post;
                    post.cid = item->cid;
                    post.indexed_at = item->indexedAt;
                    post.reference_time = reference_time;
                    m_cuePost.append(post);

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
                            // quoteしてくれたユーザーのPostの情報も取得できるようにするためキューに入れる
                            if (!m_cueGetPost.contains(item->uri)) {
                                m_cueGetPost.append(item->uri);
                            }
                            break;
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedRecordWithMedia_Main:
                            break;
                        default:
                            break;
                        }
                    } else if (item->reason == "reply" || item->reason == "mention") {
                        // quoteしてくれたユーザーのPostの情報も取得できるようにするためキューに入れる
                        if (!m_cueGetPost.contains(item->uri)) {
                            m_cueGetPost.append(item->uri);
                        }
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

                // m_post2notificationHash<cid, cid>
                //   Post側(m_postHash)からNotification側(m_notificationHash)の参照
                //
                // m_cueGetPost[cid] :
                //   Notificationの先にあるPostを取りに行く待ち行列（たぶんいくつも並列でいけるけど）
                //
                // likeとかの対象ポストの情報は入っていないので、それぞれ取得する必要あり
                // 対象ポスト情報は別途cidをキーにして保存する（2重取得と管理を避ける）
            }
            QTimer::singleShot(100, this, &NotificationListModel::displayQueuedPosts);
        }
        notification->deleteLater();
    });
    notification->setAccount(account());
    notification->listNotifications();
}

void NotificationListModel::repost(int row)
{
    if (row < 0 || row >= m_cidList.count())
        return;

    bool current = item(row, IsRepostedRole).toBool();

    if (running())
        return;
    setRunning(true);

    QPointer<NotificationListModel> aliving(this);

    RecordOperator *ope = new RecordOperator();
    connect(ope, &RecordOperator::finished,
            [=](bool success, const QString &uri, const QString &cid) {
                Q_UNUSED(cid)
                if (aliving) {
                    if (success) {
                        update(row, RepostedUriRole, uri);
                    }
                    setRunning(false);
                }
                ope->deleteLater();
            });
    ope->setAccount(account().service, account().did, account().handle, account().email,
                    account().accessJwt, account().refreshJwt);
    if (!current)
        ope->repost(item(row, CidRole).toString(), item(row, UriRole).toString());
    else
        ope->deleteRepost(item(row, RepostedUriRole).toString());
}

void NotificationListModel::like(int row)
{
    if (row < 0 || row >= m_cidList.count())
        return;

    bool current = item(row, IsLikedRole).toBool();

    if (running())
        return;
    setRunning(true);

    QPointer<NotificationListModel> aliving(this);

    RecordOperator *ope = new RecordOperator();
    connect(ope, &RecordOperator::finished,
            [=](bool success, const QString &uri, const QString &cid) {
                Q_UNUSED(cid)
                if (aliving) {
                    if (success) {
                        update(row, LikedUriRole, uri);
                    }
                    setRunning(false);
                }
                ope->deleteLater();
            });
    ope->setAccount(account().service, account().did, account().handle, account().email,
                    account().accessJwt, account().refreshJwt);
    if (!current)
        ope->like(item(row, CidRole).toString(), item(row, UriRole).toString());
    else
        ope->deleteLike(item(row, LikedUriRole).toString());
}

QHash<int, QByteArray> NotificationListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[VisibleRole] = "visible";

    roles[CidRole] = "cid";
    roles[UriRole] = "uri";
    roles[DidRole] = "did";
    roles[DisplayNameRole] = "displayName";
    roles[HandleRole] = "handle";
    roles[AvatarRole] = "avatar";
    roles[RecordTextRole] = "recordText";
    roles[RecordTextPlainRole] = "recordTextPlain";
    roles[ReplyCountRole] = "replyCount";
    roles[RepostCountRole] = "repostCount";
    roles[LikeCountRole] = "likeCount";
    roles[IndexedAtRole] = "indexedAt";

    roles[IsRepostedRole] = "isReposted";
    roles[IsLikedRole] = "isLiked";
    roles[RepostedUriRole] = "repostedUri";
    roles[LikedUriRole] = "likedUri";

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
    roles[RecordIsRepostedRole] = "recordIsReposted";
    roles[RecordIsLikedRole] = "recordIsLiked";

    return roles;
}

void NotificationListModel::finishedDisplayingQueuedPosts()
{
    QTimer::singleShot(100, this, &NotificationListModel::getPosts);
}

void NotificationListModel::getPosts()
{
    if (m_cueGetPost.isEmpty()) {
        setRunning(false);
        return;
    }

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

                //取得した個別のポストデータを表示用のcidリストのどの分か探して紐付ける
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
                        AtProtocolType::AppBskyFeedPost::Main post_quote =
                                AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                                        AtProtocolType::AppBskyFeedPost::Main>(
                                        m_notificationHash[m_cidList.at(i)].record);
                        switch (post_quote.embed_type) {
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedImages_Main:
                            break;
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedExternal_Main:
                            break;
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedRecord_Main:
                            if (new_cid.contains(
                                        post_quote.embed_AppBskyEmbedRecord_Main.record.cid)
                                || new_cid.contains(m_cidList.at(i))) {
                                emit dataChanged(index(i), index(i));
                            }
                            break;
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedRecordWithMedia_Main:
                            break;
                        default:
                            break;
                        }
                    } else if (m_notificationHash[m_cidList.at(i)].reason == "reply"
                               || m_notificationHash[m_cidList.at(i)].reason == "mention") {
                        if (new_cid.contains(m_cidList.at(i))) {
                            emit dataChanged(index(i), index(i));
                        }
                    }
                }
            }
            // 残ってたらもう1回
            QTimer::singleShot(100, this, &NotificationListModel::getPosts);
        }
        posts->deleteLater();
    });
    posts->setAccount(account());
    posts->getPosts(uris);
}

bool NotificationListModel::enableReason(const QString &reason) const
{
    if (reason == "like" && enabledLike())
        return true;
    else if (reason == "repost" && enabledRepost())
        return true;
    else if (reason == "follow" && enabledFollow())
        return true;
    else if (reason == "mention" && enabledMention())
        return true;
    else if (reason == "reply" && enabledReply())
        return true;
    else if (reason == "quote" && enabledQuote())
        return true;

    return false;
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

bool NotificationListModel::enabledLike() const
{
    return m_enabledLike;
}

void NotificationListModel::setEnabledLike(bool newEnabledLike)
{
    if (m_enabledLike == newEnabledLike)
        return;
    m_enabledLike = newEnabledLike;
    emit enabledLikeChanged();
}

bool NotificationListModel::enabledRepost() const
{
    return m_enabledRepost;
}

void NotificationListModel::setEnabledRepost(bool newEnabledRepost)
{
    if (m_enabledRepost == newEnabledRepost)
        return;
    m_enabledRepost = newEnabledRepost;
    emit enabledRepostChanged();
}

bool NotificationListModel::enabledFollow() const
{
    return m_enabledFollow;
}

void NotificationListModel::setEnabledFollow(bool newEnabledFollow)
{
    if (m_enabledFollow == newEnabledFollow)
        return;
    m_enabledFollow = newEnabledFollow;
    emit enabledFollowChanged();
}

bool NotificationListModel::enabledMention() const
{
    return m_enabledMention;
}

void NotificationListModel::setEnabledMention(bool newEnabledMention)
{
    if (m_enabledMention == newEnabledMention)
        return;
    m_enabledMention = newEnabledMention;
    emit enabledMentionChanged();
}

bool NotificationListModel::enabledReply() const
{
    return m_enabledReply;
}

void NotificationListModel::setEnabledReply(bool newEnabledReply)
{
    if (m_enabledReply == newEnabledReply)
        return;
    m_enabledReply = newEnabledReply;
    emit enabledReplyChanged();
}

bool NotificationListModel::enabledQuote() const
{
    return m_enabledQuote;
}

void NotificationListModel::setEnabledQuote(bool newEnabledQuote)
{
    if (m_enabledQuote == newEnabledQuote)
        return;
    m_enabledQuote = newEnabledQuote;
    emit enabledQuoteChanged();
}
