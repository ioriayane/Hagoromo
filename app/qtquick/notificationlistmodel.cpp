#include "notificationlistmodel.h"
#include "atprotocol/app/bsky/notification/appbskynotificationlistnotifications.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"
#include "recordoperator.h"

#include <QTimer>

using AtProtocolInterface::AppBskyFeedGetPosts;
using AtProtocolInterface::AppBskyNotificationListNotifications;

NotificationListModel::NotificationListModel(QObject *parent)
    : AtpAbstractListModel { parent },
      m_visibleLike(true),
      m_visibleRepost(true),
      m_visibleFollow(true),
      m_visibleMention(true),
      m_visibleReply(true),
      m_visibleQuote(true)
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
    else if (role == RecordTextPlainRole)
        return AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                       AtProtocolType::AppBskyFeedPost::Main>(current.record)
                .text;
    else if (role == RecordTextTranslationRole)
        return m_translations.contains(current.cid) ? m_translations[current.cid] : QString();

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

void NotificationListModel::clear()
{
    AtpAbstractListModel::clear();
    m_notificationHash.clear();
}

int NotificationListModel::indexOf(const QString &cid) const
{
    return m_cidList.indexOf(cid);
}

QString NotificationListModel::getRecordText(const QString &cid)
{
    if (!m_notificationHash.contains(cid))
        return QString();

    return AtProtocolType::LexiconsTypeUnknown::fromQVariant<AtProtocolType::AppBskyFeedPost::Main>(
                   m_notificationHash.value(cid).record)
            .text;
}

void NotificationListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    AppBskyNotificationListNotifications *notification =
            new AppBskyNotificationListNotifications(this);
    connect(notification, &AppBskyNotificationListNotifications::finished, [=](bool success) {
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
        } else {
            emit errorOccured(notification->errorMessage());
        }
        QTimer::singleShot(100, this, &NotificationListModel::displayQueuedPosts);
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

    RecordOperator *ope = new RecordOperator(this);
    connect(ope, &RecordOperator::errorOccured, this, &NotificationListModel::errorOccured);
    connect(ope, &RecordOperator::finished,
            [=](bool success, const QString &uri, const QString &cid) {
                Q_UNUSED(cid)

                if (success) {
                    update(row, RepostedUriRole, uri);
                }
                setRunning(false);
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

    RecordOperator *ope = new RecordOperator(this);
    connect(ope, &RecordOperator::errorOccured, this, &NotificationListModel::errorOccured);
    connect(ope, &RecordOperator::finished,
            [=](bool success, const QString &uri, const QString &cid) {
                Q_UNUSED(cid)
                if (success) {
                    update(row, LikedUriRole, uri);
                }
                setRunning(false);
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

    roles[CidRole] = "cid";
    roles[UriRole] = "uri";
    roles[DidRole] = "did";
    roles[DisplayNameRole] = "displayName";
    roles[HandleRole] = "handle";
    roles[AvatarRole] = "avatar";
    roles[RecordTextRole] = "recordText";
    roles[RecordTextPlainRole] = "recordTextPlain";
    roles[RecordTextTranslationRole] = "recordTextTranslation";
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

bool NotificationListModel::checkVisibility(const QString &cid)
{
    return enableReason(m_notificationHash.value(cid).reason);
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

    AppBskyFeedGetPosts *posts = new AppBskyFeedGetPosts(this);
    connect(posts, &AppBskyFeedGetPosts::finished, [=](bool success) {
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
                        if (new_cid.contains(post_quote.embed_AppBskyEmbedRecord_Main.record.cid)
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
        } else {
            emit errorOccured(posts->errorMessage());
        }
        // 残ってたらもう1回
        QTimer::singleShot(100, this, &NotificationListModel::getPosts);
        posts->deleteLater();
    });
    posts->setAccount(account());
    posts->getPosts(uris);
}

bool NotificationListModel::enableReason(const QString &reason) const
{
    if (reason == "like" && visibleLike())
        return true;
    else if (reason == "repost" && visibleRepost())
        return true;
    else if (reason == "follow" && visibleFollow())
        return true;
    else if (reason == "mention" && visibleMention())
        return true;
    else if (reason == "reply" && visibleReply())
        return true;
    else if (reason == "quote" && visibleQuote())
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

bool NotificationListModel::visibleLike() const
{
    return m_visibleLike;
}

void NotificationListModel::setVisibleLike(bool newVisibleLike)
{
    if (m_visibleLike == newVisibleLike)
        return;
    m_visibleLike = newVisibleLike;
    emit visibleLikeChanged();
    reflectVisibility();
}

bool NotificationListModel::visibleRepost() const
{
    return m_visibleRepost;
}

void NotificationListModel::setVisibleRepost(bool newVisibleRepost)
{
    if (m_visibleRepost == newVisibleRepost)
        return;
    m_visibleRepost = newVisibleRepost;
    emit visibleRepostChanged();
    reflectVisibility();
}

bool NotificationListModel::visibleFollow() const
{
    return m_visibleFollow;
}

void NotificationListModel::setVisibleFollow(bool newVisibleFollow)
{
    if (m_visibleFollow == newVisibleFollow)
        return;
    m_visibleFollow = newVisibleFollow;
    emit visibleFollowChanged();
    reflectVisibility();
}

bool NotificationListModel::visibleMention() const
{
    return m_visibleMention;
}

void NotificationListModel::setVisibleMention(bool newVisibleMention)
{
    if (m_visibleMention == newVisibleMention)
        return;
    m_visibleMention = newVisibleMention;
    emit visibleMentionChanged();
    reflectVisibility();
}

bool NotificationListModel::visibleReply() const
{
    return m_visibleReply;
}

void NotificationListModel::setVisibleReply(bool newVisibleReply)
{
    if (m_visibleReply == newVisibleReply)
        return;
    m_visibleReply = newVisibleReply;
    emit visibleReplyChanged();
    reflectVisibility();
}

bool NotificationListModel::visibleQuote() const
{
    return m_visibleQuote;
}

void NotificationListModel::setVisibleQuote(bool newVisibleQuote)
{
    if (m_visibleQuote == newVisibleQuote)
        return;
    m_visibleQuote = newVisibleQuote;
    emit visibleQuoteChanged();
    reflectVisibility();
}
