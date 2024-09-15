#include "notificationlistmodel.h"
#include "atprotocol/app/bsky/notification/appbskynotificationlistnotifications.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerators.h"
#include "atprotocol/app/bsky/graph/appbskygraphmutethread.h"
#include "atprotocol/app/bsky/graph/appbskygraphunmutethread.h"
#include "atprotocol/app/bsky/notification/appbskynotificationupdateseen.h"
#include "operation/recordoperator.h"

#include <QTimer>

using AtProtocolInterface::AppBskyFeedGetFeedGenerators;
using AtProtocolInterface::AppBskyFeedGetPosts;
using AtProtocolInterface::AppBskyGraphMuteThread;
using AtProtocolInterface::AppBskyGraphUnmuteThread;
using AtProtocolInterface::AppBskyNotificationListNotifications;
using AtProtocolInterface::AppBskyNotificationUpdateSeen;
using namespace AtProtocolType::AppBskyFeedDefs;

NotificationListModel::NotificationListModel(QObject *parent)
    : AtpAbstractListModel { parent, true },
      m_hasUnread(false),
      m_visibleLike(true),
      m_visibleRepost(true),
      m_visibleFollow(true),
      m_visibleMention(true),
      m_visibleReply(true),
      m_visibleQuote(true),
      m_updateSeenNotification(true),
      m_aggregateReactions(true)
{
    m_toEmbedVideoRoles[HasVideoRole] = AtpAbstractListModel::EmbedVideoRoles::HasVideoRole;
    m_toEmbedVideoRoles[VideoPlaylistRole] =
            AtpAbstractListModel::EmbedVideoRoles::VideoPlaylistRole;
    m_toEmbedVideoRoles[VideoThumbRole] = AtpAbstractListModel::EmbedVideoRoles::VideoThumbRole;
    m_toEmbedVideoRoles[VideoAltRole] = AtpAbstractListModel::EmbedVideoRoles::VideoAltRole;

    m_toQuoteRecordVideoRoles[QuoteRecordHasVideoRole] =
            AtpAbstractListModel::EmbedVideoRoles::HasVideoRole;
    m_toQuoteRecordVideoRoles[QuoteRecordVideoPlaylistRole] =
            AtpAbstractListModel::EmbedVideoRoles::VideoPlaylistRole;
    m_toQuoteRecordVideoRoles[QuoteRecordVideoThumbRole] =
            AtpAbstractListModel::EmbedVideoRoles::VideoThumbRole;
    m_toQuoteRecordVideoRoles[QuoteRecordVideoAltRole] =
            AtpAbstractListModel::EmbedVideoRoles::VideoAltRole;

    m_toExternalLinkRoles[HasExternalLinkRole] =
            AtpAbstractListModel::ExternalLinkRoles::HasExternalLinkRole;
    m_toExternalLinkRoles[ExternalLinkUriRole] =
            AtpAbstractListModel::ExternalLinkRoles::ExternalLinkUriRole;
    m_toExternalLinkRoles[ExternalLinkTitleRole] =
            AtpAbstractListModel::ExternalLinkRoles::ExternalLinkTitleRole;
    m_toExternalLinkRoles[ExternalLinkDescriptionRole] =
            AtpAbstractListModel::ExternalLinkRoles::ExternalLinkDescriptionRole;
    m_toExternalLinkRoles[ExternalLinkThumbRole] =
            AtpAbstractListModel::ExternalLinkRoles::ExternalLinkThumbRole;

    m_toFeedGeneratorRoles[HasFeedGeneratorRole] =
            AtpAbstractListModel::FeedGeneratorRoles::HasFeedGeneratorRole;
    m_toFeedGeneratorRoles[FeedGeneratorUriRole] =
            AtpAbstractListModel::FeedGeneratorRoles::FeedGeneratorUriRole;
    m_toFeedGeneratorRoles[FeedGeneratorCreatorHandleRole] =
            AtpAbstractListModel::FeedGeneratorRoles::FeedGeneratorCreatorHandleRole;
    m_toFeedGeneratorRoles[FeedGeneratorDisplayNameRole] =
            AtpAbstractListModel::FeedGeneratorRoles::FeedGeneratorDisplayNameRole;
    m_toFeedGeneratorRoles[FeedGeneratorLikeCountRole] =
            AtpAbstractListModel::FeedGeneratorRoles::FeedGeneratorLikeCountRole;
    m_toFeedGeneratorRoles[FeedGeneratorAvatarRole] =
            AtpAbstractListModel::FeedGeneratorRoles::FeedGeneratorAvatarRole;

    m_toListLinkRoles[HasListLinkRole] = AtpAbstractListModel::ListLinkRoles::HasListLinkRole;
    m_toListLinkRoles[ListLinkUriRole] = AtpAbstractListModel::ListLinkRoles::ListLinkUriRole;
    m_toListLinkRoles[ListLinkCreatorHandleRole] =
            AtpAbstractListModel::ListLinkRoles::ListLinkCreatorHandleRole;
    m_toListLinkRoles[ListLinkDisplayNameRole] =
            AtpAbstractListModel::ListLinkRoles::ListLinkDisplayNameRole;
    m_toListLinkRoles[ListLinkDescriptionRole] =
            AtpAbstractListModel::ListLinkRoles::ListLinkDescriptionRole;
    m_toListLinkRoles[ListLinkAvatarRole] = AtpAbstractListModel::ListLinkRoles::ListLinkAvatarRole;
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
    else if (role == MutedRole)
        return current.author.viewer.muted;
    else if (role == RecordTextRole)
        return AtProtocolType::LexiconsTypeUnknown::copyRecordText(current.record);
    else if (role == RecordTextPlainRole)
        return AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                       AtProtocolType::AppBskyFeedPost::Main>(current.record)
                .text;
    else if (role == RecordTextTranslationRole)
        return getTranslation(current.cid);
    else if (role == IndexedAtRole)
        return AtProtocolType::LexiconsTypeUnknown::formatDateTime(current.indexedAt);
    else if (role == EmbedImagesRole) {
        if (m_postHash.contains(current.cid))
            return AtProtocolType::LexiconsTypeUnknown::copyImagesFromPostView(
                    m_postHash[current.cid],
                    AtProtocolType::LexiconsTypeUnknown::CopyImageType::Thumb);
        else
            return QStringList();
    } else if (role == EmbedImagesFullRole) {
        if (m_postHash.contains(current.cid))
            return AtProtocolType::LexiconsTypeUnknown::copyImagesFromPostView(
                    m_postHash[current.cid],
                    AtProtocolType::LexiconsTypeUnknown::CopyImageType::FullSize);
        else
            return QStringList();
    } else if (role == EmbedImagesAltRole) {
        if (m_postHash.contains(current.cid))
            return AtProtocolType::LexiconsTypeUnknown::copyImagesFromPostView(
                    m_postHash[current.cid],
                    AtProtocolType::LexiconsTypeUnknown::CopyImageType::Alt);
        else
            return QStringList();

        //----------------------------------------
    } else if (role == ReplyCountRole) {
        if (m_postHash.contains(current.cid))
            return m_postHash[current.cid].replyCount;
        else
            return 0;
    } else if (role == RepostCountRole) {
        if (m_postHash.contains(current.cid))
            return m_postHash[current.cid].repostCount;
        else
            return 0;
    } else if (role == QuoteCountRole) {
        if (m_postHash.contains(current.cid))
            return m_postHash[current.cid].quoteCount;
        else
            return 0;
    } else if (role == LikeCountRole) {
        if (m_postHash.contains(current.cid))
            return m_postHash[current.cid].likeCount;
        else
            return 0;
    } else if (role == ReplyDisabledRole) {
        if (m_postHash.contains(current.cid))
            return m_postHash[current.cid].viewer.replyDisabled;
        else
            return false;
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
    } else if (role == ThreadMutedRole) {
        if (m_postHash.contains(current.cid))
            return m_postHash[current.cid].viewer.threadMuted;
        else
            return false;
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
    } else if (role == RunningRepostRole) {
        return !current.cid.isEmpty() && (current.cid == m_runningRepostCid);
    } else if (role == RunningLikeRole) {
        return !current.cid.isEmpty() && (current.cid == m_runningLikeCid);

    } else if (role == AggregatedAvatarsRole || role == AggregatedDisplayNamesRole
               || role == AggregatedDidsRole || role == AggregatedHandlesRole
               || role == AggregatedIndexedAtsRole) {
        return getAggregatedItems(current, role);

    } else if (role == ReplyRootCidRole) {
        return AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                       AtProtocolType::AppBskyFeedPost::Main>(current.record)
                .reply.root.cid;
    } else if (role == ReplyRootUriRole) {
        return AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                       AtProtocolType::AppBskyFeedPost::Main>(current.record)
                .reply.root.uri;

    } else if (role == UserFilterMatchedRole) {
        return getContentFilterMatched(current.author.labels, false);
    } else if (role == UserFilterMessageRole) {
        return getContentFilterMessage(current.author.labels, false);
    } else if (role == ContentFilterMatchedRole) {
        if (hideByMutedWords(current.cid, current.author.did)) {
            return true;
        } else {
            return getContentFilterMatched(current.labels, false);
        }
    } else if (role == ContentFilterMessageRole) {
        if (hideByMutedWords(current.cid, current.author.did)) {
            return tr("Post hidden by muted word");
        } else {
            return getContentFilterMessage(current.labels, false);
        }
    } else if (role == ContentMediaFilterMatchedRole) {
        return getContentFilterMatched(current.labels, true);
    } else if (role == ContentMediaFilterMessageRole) {
        return getContentFilterMessage(current.labels, true);

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
        } else if (current.reason == "starterpack-joined") {
            return NotificationListModelReason::ReasonStaterPack;
        } else {
            return NotificationListModelReason::ReasonUnknown;
        }

    } else if (m_toEmbedVideoRoles.contains(role)) {
        return getEmbedVideoItem(m_postHash.value(current.cid), m_toEmbedVideoRoles[role]);

    } else if (m_toExternalLinkRoles.contains(role)) {
        return AtpAbstractListModel::getExternalLinkItem(m_postHash.value(current.cid),
                                                         m_toExternalLinkRoles[role]);

    } else if (m_toFeedGeneratorRoles.contains(role)
               && (current.reason == "mention" || current.reason == "reply"
                   || current.reason == "quote")) {
        return getFeedGeneratorItem(m_postHash.value(current.cid), m_toFeedGeneratorRoles[role]);

    } else if (m_toListLinkRoles.contains(role)) {
        return getListLinkItem(m_postHash.value(current.cid), m_toListLinkRoles[role]);

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
                if (!post.embed_AppBskyEmbedRecordWithMedia_Main.record.isNull()) {
                    record_cid = post.embed_AppBskyEmbedRecordWithMedia_Main.record->record.cid;
                    //                    record_cid = current.reasonSubject;
                }
                break;
            default:
                break;
            }
        }

        //----------------------------------------
        if (role == QuoteRecordCidRole) {
            if (m_postHash.contains(record_cid))
                return m_postHash[record_cid].cid;
            else
                return QString();
        } else if (role == QuoteRecordUriRole) {
            if (m_postHash.contains(record_cid))
                return m_postHash[record_cid].uri;
            else
                return QString();
        } else if (role == QuoteRecordDisplayNameRole) {
            if (m_postHash.contains(record_cid))
                return m_postHash[record_cid].author.displayName;
            else
                return QString();
        } else if (role == QuoteRecordHandleRole) {
            if (m_postHash.contains(record_cid))
                return m_postHash[record_cid].author.handle;
            else
                return QString();
        } else if (role == QuoteRecordAvatarRole) {
            if (m_postHash.contains(record_cid))
                return m_postHash[record_cid].author.avatar;
            else
                return QString();
        } else if (role == QuoteRecordIndexedAtRole) {
            if (m_postHash.contains(record_cid))
                return AtProtocolType::LexiconsTypeUnknown::formatDateTime(
                        m_postHash[record_cid].indexedAt);
            else
                return QString();
        } else if (role == QuoteRecordRecordTextRole) {
            if (m_postHash.contains(record_cid))
                return AtProtocolType::LexiconsTypeUnknown::copyRecordText(
                        m_postHash[record_cid].record);
            else
                return QString();
        } else if (role == QuoteRecordEmbedImagesRole) {
            if (m_postHash.contains(record_cid))
                return AtProtocolType::LexiconsTypeUnknown::copyImagesFromPostView(
                        m_postHash[record_cid],
                        AtProtocolType::LexiconsTypeUnknown::CopyImageType::Thumb);
            else
                return QStringList();
        } else if (role == QuoteRecordEmbedImagesFullRole) {
            if (m_postHash.contains(record_cid))
                return AtProtocolType::LexiconsTypeUnknown::copyImagesFromPostView(
                        m_postHash[record_cid],
                        AtProtocolType::LexiconsTypeUnknown::CopyImageType::FullSize);
            else
                return QStringList();
        } else if (role == QuoteRecordEmbedImagesAltRole) {
            if (m_postHash.contains(record_cid))
                return AtProtocolType::LexiconsTypeUnknown::copyImagesFromPostView(
                        m_postHash[record_cid],
                        AtProtocolType::LexiconsTypeUnknown::CopyImageType::Alt);
            else
                return QStringList();
        } else if (role == QuoteRecordIsRepostedRole) {
            if (m_postHash.contains(record_cid))
                return m_postHash[record_cid].viewer.repost.contains(account().did);
            else
                return false;
        } else if (role == QuoteRecordIsLikedRole) {
            if (m_postHash.contains(record_cid))
                return m_postHash[record_cid].viewer.like.contains(account().did);
            else
                return false;

        } else if (m_toQuoteRecordVideoRoles.contains(role)) {
            if (m_postHash.contains(record_cid))
                return getEmbedVideoItem(m_postHash.value(record_cid),
                                         m_toQuoteRecordVideoRoles[role]);
            else
                return QString();

        } else if (role == HasFeedGeneratorRole) { // カスタムフィードに対するいいね
            if (m_feedGeneratorHash.contains(record_cid)) {
                return true;
            } else {
                return false;
            }
        } else if (role == FeedGeneratorUriRole) {
            if (m_feedGeneratorHash.contains(record_cid)) {
                return m_feedGeneratorHash[record_cid].uri;
            } else {
                return QString();
            }
        } else if (role == FeedGeneratorCreatorHandleRole) {
            if (m_feedGeneratorHash.contains(record_cid)) {
                return m_feedGeneratorHash[record_cid].creator.handle;
            } else {
                return QString();
            }
        } else if (role == FeedGeneratorDisplayNameRole) {
            if (m_feedGeneratorHash.contains(record_cid)) {
                return m_feedGeneratorHash[record_cid].displayName;
            } else {
                return QString();
            }
        } else if (role == FeedGeneratorLikeCountRole) {
            if (m_feedGeneratorHash.contains(record_cid)) {
                return m_feedGeneratorHash[record_cid].likeCount;
            } else {
                return QString();
            }
        } else if (role == FeedGeneratorAvatarRole) {
            if (m_feedGeneratorHash.contains(record_cid)) {
                return m_feedGeneratorHash[record_cid].avatar;
            } else {
                return QString();
            }
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
    } else if (role == ThreadMutedRole) {
        if (m_postHash.contains(current.cid)) {
            qDebug().noquote() << "update Thread Mute" << row
                               << m_postHash[current.cid].viewer.threadMuted << "->"
                               << value.toString();
            if (m_postHash[current.cid].viewer.threadMuted != value.toBool()) {
                m_postHash[current.cid].viewer.threadMuted = value.toBool();
                emit dataChanged(index(row), index(row), QVector<int>() << role);
            }
        }
    } else if (role == RunningRepostRole) {
        if (value.toBool()) {
            m_runningRepostCid = current.cid;
        } else {
            m_runningRepostCid.clear();
        }
        emit dataChanged(index(row), index(row));
    } else if (role == RunningLikeRole) {
        if (value.toBool()) {
            m_runningLikeCid = current.cid;
        } else {
            m_runningLikeCid.clear();
        }
        emit dataChanged(index(row), index(row));
    }

    return;
}

void NotificationListModel::clear()
{
    AtpAbstractListModel::clear();
    m_notificationHash.clear();
    m_liked2Notification.clear();
    m_reposted2Notification.clear();
    m_follow2Notification.clear();
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

QString NotificationListModel::getItemOfficialUrl(int row) const
{
    return atUriToOfficialUrl(item(row, UriRole).toString(), QStringLiteral("post"));
}

bool NotificationListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyNotificationListNotifications *notification =
                new AppBskyNotificationListNotifications(this);
        connect(notification, &AppBskyNotificationListNotifications::finished, [=](bool success) {
            m_hasUnread = false;
            if (success) {
                QDateTime reference_time = QDateTime::currentDateTimeUtc();

                if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                    m_cursor = notification->cursor();
                }
                for (auto item = notification->notificationsList().crbegin();
                     item != notification->notificationsList().crend(); item++) {
                    m_notificationHash[item->cid] = *item;

                    PostCueItem post;
                    post.cid = item->cid;
                    post.indexed_at = item->indexedAt;
                    post.reference_time = reference_time;
                    post.reason = item->reason;
                    m_cuePost.append(post);

                    if (!item->isRead) {
                        m_hasUnread = true;
                    }

                    if (item->reason == "like") {
                        if (item->reasonSubject.contains("/app.bsky.feed.generator/")) {
                            appendGetFeedGeneratorCue<AtProtocolType::AppBskyFeedLike::Main>(
                                    item->record);
                        } else {
                            appendGetPostCue<AtProtocolType::AppBskyFeedLike::Main>(item->record);
                        }
                    } else if (item->reason == "repost") {
                        if (item->reasonSubject.contains("/app.bsky.feed.generator/")) {
                            appendGetFeedGeneratorCue<AtProtocolType::AppBskyFeedRepost::Main>(
                                    item->record);
                        } else {
                            appendGetPostCue<AtProtocolType::AppBskyFeedRepost::Main>(item->record);
                        }
                    } else if (item->reason == "quote") {
                        AtProtocolType::AppBskyFeedPost::Main quote_post =
                                AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                                        AtProtocolType::AppBskyFeedPost::Main>(item->record);
                        switch (quote_post.embed_type) {
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedImages_Main:
                            break;
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedExternal_Main:
                            break;
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedRecord_Main:
                            if (!quote_post.embed_AppBskyEmbedRecord_Main.record.cid.isEmpty()
                                && !m_cueGetPost.contains(
                                        quote_post.embed_AppBskyEmbedRecord_Main.record.uri)) {
                                m_cueGetPost.append(
                                        quote_post.embed_AppBskyEmbedRecord_Main.record.uri);
                            }
                            // quoteしてくれたユーザーのPostの情報も取得できるようにするためキューに入れる
                            if (!m_cueGetPost.contains(item->uri)) {
                                m_cueGetPost.append(item->uri);
                            }
                            break;
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedRecordWithMedia_Main:
                            if (quote_post.embed_AppBskyEmbedRecordWithMedia_Main.record.isNull()) {
                            } else if (quote_post.embed_AppBskyEmbedRecordWithMedia_Main.record
                                               ->record.uri.contains("/app.bsky.feed.generator/")) {
                                if (!m_cueGetFeedGenerator.contains(
                                            quote_post.embed_AppBskyEmbedRecordWithMedia_Main
                                                    .record->record.uri)) {
                                    m_cueGetFeedGenerator.append(
                                            quote_post.embed_AppBskyEmbedRecordWithMedia_Main
                                                    .record->record.uri);
                                }
                            } else if (!quote_post.embed_AppBskyEmbedRecordWithMedia_Main.record
                                                ->record.uri.isEmpty()
                                       && !m_cueGetPost.contains(
                                               quote_post.embed_AppBskyEmbedRecordWithMedia_Main
                                                       .record->record.uri)) {
                                m_cueGetPost.append(
                                        quote_post.embed_AppBskyEmbedRecordWithMedia_Main.record
                                                ->record.uri);
                            }
                            // quoteしてくれたユーザーのPostの情報も取得できるようにするためキューに入れる
                            if (!m_cueGetPost.contains(item->uri)) {
                                m_cueGetPost.append(item->uri);
                            }
                            break;
                        default:
                            break;
                        }
                    } else if (item->reason == "reply" || item->reason == "mention") {
                        // quoteしてくれたユーザーのPostの情報も取得できるようにするためキューに入れる
                        if (!m_cueGetPost.contains(item->uri)) {
                            m_cueGetPost.append(item->uri);
                        }
                    } else if (item->reason == "starterpack-joined") {
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
                emit errorOccured(notification->errorCode(), notification->errorMessage());
            }
            QTimer::singleShot(10, this, &NotificationListModel::displayQueuedPosts);
            notification->deleteLater();
        });
        notification->setAccount(account());
        notification->setLabelers(labelerDids());
        notification->listNotifications(0, false, QString(), QString());
    });
    return true;
}

bool NotificationListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyNotificationListNotifications *notification =
                new AppBskyNotificationListNotifications(this);
        connect(notification, &AppBskyNotificationListNotifications::finished, [=](bool success) {
            m_hasUnread = false;
            if (success) {
                QDateTime reference_time = QDateTime::currentDateTimeUtc();

                m_cursor = notification->cursor();

                for (auto item = notification->notificationsList().crbegin();
                     item != notification->notificationsList().crend(); item++) {
                    m_notificationHash[item->cid] = *item;

                    PostCueItem post;
                    post.cid = item->cid;
                    post.indexed_at = item->indexedAt;
                    post.reference_time = reference_time;
                    post.reason = item->reason;
                    m_cuePost.append(post);

                    if (!item->isRead) {
                        m_hasUnread = true;
                    }

                    if (item->reason == "like") {
                        if (item->reasonSubject.contains("/app.bsky.feed.generator/")) {
                            appendGetFeedGeneratorCue<AtProtocolType::AppBskyFeedLike::Main>(
                                    item->record);
                        } else {
                            appendGetPostCue<AtProtocolType::AppBskyFeedLike::Main>(item->record);
                        }
                    } else if (item->reason == "repost") {
                        if (item->reasonSubject.contains("/app.bsky.feed.generator/")) {
                            appendGetFeedGeneratorCue<AtProtocolType::AppBskyFeedRepost::Main>(
                                    item->record);
                        } else {
                            appendGetPostCue<AtProtocolType::AppBskyFeedRepost::Main>(item->record);
                        }
                    } else if (item->reason == "quote") {
                        AtProtocolType::AppBskyFeedPost::Main quote_post =
                                AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                                        AtProtocolType::AppBskyFeedPost::Main>(item->record);
                        switch (quote_post.embed_type) {
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedImages_Main:
                            break;
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedExternal_Main:
                            break;
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedRecord_Main:
                            if (quote_post.embed_AppBskyEmbedRecord_Main.record.uri.contains(
                                        "/app.bsky.feed.generator/")) {
                                if (!m_cueGetFeedGenerator.contains(
                                            quote_post.embed_AppBskyEmbedRecord_Main.record.uri)) {
                                    m_cueGetFeedGenerator.append(
                                            quote_post.embed_AppBskyEmbedRecord_Main.record.uri);
                                }
                            } else if (!quote_post.embed_AppBskyEmbedRecord_Main.record.cid
                                                .isEmpty()
                                       && !m_cueGetPost.contains(
                                               quote_post.embed_AppBskyEmbedRecord_Main.record
                                                       .uri)) {
                                m_cueGetPost.append(
                                        quote_post.embed_AppBskyEmbedRecord_Main.record.uri);
                            }
                            // quoteしてくれたユーザーのPostの情報も取得できるようにするためキューに入れる
                            if (!m_cueGetPost.contains(item->uri)) {
                                m_cueGetPost.append(item->uri);
                            }
                            break;
                        case AtProtocolType::AppBskyFeedPost::MainEmbedType::
                                embed_AppBskyEmbedRecordWithMedia_Main:
                            if (quote_post.embed_AppBskyEmbedRecordWithMedia_Main.record.isNull()) {
                            } else if (quote_post.embed_AppBskyEmbedRecordWithMedia_Main.record
                                               ->record.uri.contains("/app.bsky.feed.generator/")) {
                                if (!m_cueGetFeedGenerator.contains(
                                            quote_post.embed_AppBskyEmbedRecordWithMedia_Main
                                                    .record->record.uri)) {
                                    m_cueGetFeedGenerator.append(
                                            quote_post.embed_AppBskyEmbedRecordWithMedia_Main
                                                    .record->record.uri);
                                }
                            } else if (!quote_post.embed_AppBskyEmbedRecordWithMedia_Main.record
                                                ->record.uri.isEmpty()
                                       && !m_cueGetPost.contains(
                                               quote_post.embed_AppBskyEmbedRecordWithMedia_Main
                                                       .record->record.uri)) {
                                m_cueGetPost.append(
                                        quote_post.embed_AppBskyEmbedRecordWithMedia_Main.record
                                                ->record.uri);
                            }
                            // quoteしてくれたユーザーのPostの情報も取得できるようにするためキューに入れる
                            if (!m_cueGetPost.contains(item->uri)) {
                                m_cueGetPost.append(item->uri);
                            }
                            break;
                        default:
                            break;
                        }
                    } else if (item->reason == "reply" || item->reason == "mention") {
                        // quoteしてくれたユーザーのPostの情報も取得できるようにするためキューに入れる
                        if (!m_cueGetPost.contains(item->uri)) {
                            m_cueGetPost.append(item->uri);
                        }
                    } else if (item->reason == "starterpack-joined") {
                    }
                }
            } else {
                emit errorOccured(notification->errorCode(), notification->errorMessage());
            }
            QTimer::singleShot(10, this, &NotificationListModel::displayQueuedPostsNext);
            notification->deleteLater();
        });
        notification->setAccount(account());
        notification->setLabelers(labelerDids());
        notification->listNotifications(0, false, m_cursor, QString());
    });
    return true;
}

bool NotificationListModel::repost(int row)
{
    if (row < 0 || row >= m_cidList.count())
        return false;

    bool current = item(row, IsRepostedRole).toBool();

    if (runningRepost(row))
        return false;
    setRunningRepost(row, true);

    RecordOperator *ope = new RecordOperator(this);
    connect(ope, &RecordOperator::errorOccured, this, &NotificationListModel::errorOccured);
    connect(ope, &RecordOperator::finished,
            [=](bool success, const QString &uri, const QString &cid) {
                Q_UNUSED(cid)

                if (success) {
                    update(row, RepostedUriRole, uri);
                }
                setRunningRepost(row, false);
                ope->deleteLater();
            });
    ope->setAccount(account().service, account().did, account().handle, account().email,
                    account().accessJwt, account().refreshJwt);
    if (!current)
        ope->repost(item(row, CidRole).toString(), item(row, UriRole).toString());
    else
        ope->deleteRepost(item(row, RepostedUriRole).toString());

    return true;
}

bool NotificationListModel::like(int row)
{
    if (row < 0 || row >= m_cidList.count())
        return false;

    bool current = item(row, IsLikedRole).toBool();

    if (runningLike(row))
        return false;
    setRunningLike(row, true);

    RecordOperator *ope = new RecordOperator(this);
    connect(ope, &RecordOperator::errorOccured, this, &NotificationListModel::errorOccured);
    connect(ope, &RecordOperator::finished,
            [=](bool success, const QString &uri, const QString &cid) {
                Q_UNUSED(cid)
                if (success) {
                    update(row, LikedUriRole, uri);
                }
                setRunningLike(row, false);
                ope->deleteLater();
            });
    ope->setAccount(account().service, account().did, account().handle, account().email,
                    account().accessJwt, account().refreshJwt);
    if (!current)
        ope->like(item(row, CidRole).toString(), item(row, UriRole).toString());
    else
        ope->deleteLike(item(row, LikedUriRole).toString());

    return true;
}

bool NotificationListModel::muteThread(int row)
{
    if (row < 0 || row >= m_cidList.count())
        return false;

    if (!m_postHash.contains(m_cidList.at(row)))
        return false;

    const auto &current = m_postHash.value(m_cidList.at(row));

    const AtProtocolType::AppBskyFeedPost::Main record =
            AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                    AtProtocolType::AppBskyFeedPost::Main>(current.record);
    QString root_uri = record.reply.root.uri;
    if (root_uri.isEmpty() || !root_uri.startsWith("at://")) {
        return false;
    }

    bool muted = item(row, ThreadMutedRole).toBool();
    if (muted) {
        // true -> false
        AppBskyGraphUnmuteThread *thread = new AppBskyGraphUnmuteThread(this);
        connect(thread, &AppBskyGraphUnmuteThread::finished, this, [=](bool success) {
            if (success) {
                getPostThreadCids(root_uri,
                                  [=](const QStringList &cids) { updateMuteThread(cids, false); });
            } else {
                emit errorOccured(thread->errorCode(), thread->errorMessage());
            }
            thread->deleteLater();
        });
        thread->setAccount(account());
        thread->unmuteThread(root_uri);
    } else {
        // false -> true
        AppBskyGraphMuteThread *thread = new AppBskyGraphMuteThread(this);
        connect(thread, &AppBskyGraphMuteThread::finished, this, [=](bool success) {
            if (success) {
                getPostThreadCids(root_uri,
                                  [=](const QStringList &cids) { updateMuteThread(cids, true); });
            } else {
                emit errorOccured(thread->errorCode(), thread->errorMessage());
            }
            thread->deleteLater();
        });
        thread->setAccount(account());
        thread->muteThread(root_uri);
    }
    return true;
}

bool NotificationListModel::detachQuote(int row)
{
    return true;
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
    roles[MutedRole] = "muted";
    roles[RecordTextRole] = "recordText";
    roles[RecordTextPlainRole] = "recordTextPlain";
    roles[RecordTextTranslationRole] = "recordTextTranslation";
    roles[ReplyCountRole] = "replyCount";
    roles[RepostCountRole] = "repostCount";
    roles[QuoteCountRole] = "quoteCount";
    roles[LikeCountRole] = "likeCount";
    roles[ReplyDisabledRole] = "replyDisabled";
    roles[IndexedAtRole] = "indexedAt";
    roles[EmbedImagesRole] = "embedImages";
    roles[EmbedImagesFullRole] = "embedImagesFull";
    roles[EmbedImagesAltRole] = "embedImagesAlt";

    roles[IsRepostedRole] = "isReposted";
    roles[IsLikedRole] = "isLiked";
    roles[ThreadMutedRole] = "threadMuted";
    roles[RepostedUriRole] = "repostedUri";
    roles[LikedUriRole] = "likedUri";
    roles[RunningRepostRole] = "runningRepost";
    roles[RunningLikeRole] = "runningLike";

    roles[AggregatedAvatarsRole] = "aggregatedAvatars";
    roles[AggregatedDisplayNamesRole] = "aggregatedDisplayNames";
    roles[AggregatedDidsRole] = "aggregatedDids";
    roles[AggregatedHandlesRole] = "aggregatedHandles";
    roles[AggregatedIndexedAtsRole] = "aggregatedIndexedAts";

    roles[ReasonRole] = "reason";

    roles[QuoteRecordCidRole] = "quoteRecordCid";
    roles[QuoteRecordUriRole] = "quoteRecordUri";
    roles[QuoteRecordDisplayNameRole] = "quoteRecordDisplayName";
    roles[QuoteRecordHandleRole] = "quoteRecordHandle";
    roles[QuoteRecordAvatarRole] = "quoteRecordAvatar";
    roles[QuoteRecordIndexedAtRole] = "quoteRecordIndexedAt";
    roles[QuoteRecordRecordTextRole] = "quoteRecordRecordText";
    roles[QuoteRecordEmbedImagesRole] = "quoteRecordEmbedImages";
    roles[QuoteRecordEmbedImagesFullRole] = "quoteRecordEmbedImagesFull";
    roles[QuoteRecordEmbedImagesAltRole] = "quoteRecordEmbedImagesAlt";
    roles[QuoteRecordIsRepostedRole] = "quoteRecordIsReposted";
    roles[QuoteRecordIsLikedRole] = "quoteRecordIsLiked";
    roles[QuoteRecordHasVideoRole] = "quoteRecordHasVideo";
    roles[QuoteRecordVideoPlaylistRole] = "quoteRecordVideoPlaylist";
    roles[QuoteRecordVideoThumbRole] = "quoteRecordVideoThumb";
    roles[QuoteRecordVideoAltRole] = "quoteRecordVideoAlt";

    roles[HasVideoRole] = "hasVideo";
    roles[VideoPlaylistRole] = "videoPlaylist";
    roles[VideoThumbRole] = "videoThumbUri";
    roles[VideoAltRole] = "videoAlt";

    roles[HasExternalLinkRole] = "hasExternalLink";
    roles[ExternalLinkUriRole] = "externalLinkUri";
    roles[ExternalLinkTitleRole] = "externalLinkTitle";
    roles[ExternalLinkDescriptionRole] = "externalLinkDescription";
    roles[ExternalLinkThumbRole] = "externalLinkThumb";

    roles[HasFeedGeneratorRole] = "hasFeedGenerator";
    roles[FeedGeneratorUriRole] = "feedGeneratorUri";
    roles[FeedGeneratorCreatorHandleRole] = "feedGeneratorCreatorHandle";
    roles[FeedGeneratorDisplayNameRole] = "feedGeneratorDisplayName";
    roles[FeedGeneratorLikeCountRole] = "feedGeneratorLikeCount";
    roles[FeedGeneratorAvatarRole] = "feedGeneratorAvatar";

    roles[HasListLinkRole] = "hasListLink";
    roles[ListLinkUriRole] = "listLinkUri";
    roles[ListLinkCreatorHandleRole] = "listLinkCreatorHandle";
    roles[ListLinkDisplayNameRole] = "listLinkDisplayName";
    roles[ListLinkDescriptionRole] = "listLinkDescription";
    roles[ListLinkAvatarRole] = "listLinkAvatar";

    roles[ReplyRootCidRole] = "replyRootCid";
    roles[ReplyRootUriRole] = "replyRootUri";

    roles[UserFilterMatchedRole] = "userFilterMatched";
    roles[UserFilterMessageRole] = "userFilterMessage";
    roles[ContentFilterMatchedRole] = "contentFilterMatched";
    roles[ContentFilterMessageRole] = "contentFilterMessage";
    roles[ContentMediaFilterMatchedRole] = "contentMediaFilterMatched";
    roles[ContentMediaFilterMessageRole] = "contentMediaFilterMessage";

    return roles;
}

void NotificationListModel::finishedDisplayingQueuedPosts()
{
    QTimer::singleShot(100, this, &NotificationListModel::getPosts);
}

bool NotificationListModel::checkVisibility(const QString &cid)
{
    if (!m_notificationHash.contains(cid))
        return true;

    const auto &current = m_notificationHash.value(cid);

    // ミュートワードの判定
    if (cachePostsContainingMutedWords(
                current.cid,
                AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                        AtProtocolType::AppBskyFeedPost::Main>(current.record))) {
        if (current.author.did != account().did && !visibleContainingMutedWord()) {
            return false;
        }
    }

    if (!enableReason(current.reason))
        return false;

    for (const auto &label : current.author.labels) {
        if (visibilityBylabeler(label.val, false, label.src) == ConfigurableLabelStatus::Hide) {
            qDebug() << "Hide notification by user's label. " << current.author.handle << cid;
            return false;
        }
    }
    for (const auto &label : current.labels) {
        if (visibilityBylabeler(label.val, true, label.src) == ConfigurableLabelStatus::Hide) {
            qDebug() << "Hide notification by post's label. " << current.author.handle << cid;
            return false;
        }
    }

    return true;
}

void NotificationListModel::displayQueuedPosts()
{
    int interval = displayInterval();
    bool batch_mode = (m_originalCidList.isEmpty() || interval == 0);

    while (!m_cuePost.isEmpty()) {
        const PostCueItem &post = m_cuePost.front();
        bool visible = checkVisibility(post.cid);
        if (!visible)
            interval = 0;

        if (m_originalCidList.contains(post.cid)) {
            // リストは更新しないでデータのみ入れ替える
            // 更新をUIに通知
            // （取得できた範囲でしか更新できないのだけど・・・）
            interval = 0;
            int r = m_cidList.indexOf(post.cid);
            if (r >= 0) {
                if (visible) {
                    emit dataChanged(index(r), index(r));
                } else {
                    beginRemoveRows(QModelIndex(), r, r);
                    m_cidList.removeAt(r);
                    endRemoveRows();
                }
            } else if (aggregated(post.cid)) {
                // 集約済み
            } else {
                int r = searchInsertPosition(post.cid);
                if (visible && r >= 0) {
                    // 復活させる
                    aggregateQueuedPosts(post.cid);
                    beginInsertRows(QModelIndex(), r, r);
                    m_cidList.insert(r, post.cid);
                    endInsertRows();
                }
            }
        } else {
            if (visible) {
                aggregateQueuedPosts(post.cid);
                beginInsertRows(QModelIndex(), 0, 0);
                m_cidList.insert(0, post.cid);
                endInsertRows();
            }
            m_originalCidList.insert(0, post.cid);
        }

        m_cuePost.pop_front();

        if (!batch_mode)
            break;
    }

    if (!m_cuePost.isEmpty()) {
        QTimer::singleShot(interval, this, &NotificationListModel::displayQueuedPosts);
    } else {
        finishedDisplayingQueuedPosts();
    }
}

void NotificationListModel::displayQueuedPostsNext()
{
    while (!m_cuePost.isEmpty()) {
        const PostCueItem &post = m_cuePost.back();
        bool visible = checkVisibility(post.cid);

        if (m_originalCidList.contains(post.cid)) {
            // リストは更新しないでデータのみ入れ替える
            // 更新をUIに通知
            // （取得できた範囲でしか更新できないのだけど・・・）
            int r = m_cidList.indexOf(post.cid);
            if (r >= 0) {
                if (visible) {
                    emit dataChanged(index(r), index(r));
                } else {
                    beginRemoveRows(QModelIndex(), r, r);
                    m_cidList.removeAt(r);
                    endRemoveRows();
                }
            } else if (aggregated(post.cid)) {
                // 集約済み
            } else {
                int r = searchInsertPosition(post.cid);
                if (visible && r >= 0) {
                    // 復活させる
                    beginInsertRows(QModelIndex(), r, r);
                    m_cidList.insert(r, post.cid);
                    endInsertRows();
                }
            }
        } else {
            if (visible) {
                if (aggregateQueuedPosts(post.cid, true)) {
                    beginInsertRows(QModelIndex(), m_cidList.count(), m_cidList.count());
                    m_cidList.append(post.cid);
                    endInsertRows();
                }
            }
            m_originalCidList.append(post.cid);
        }

        m_cuePost.pop_back();
    }

    finishedDisplayingQueuedPosts();
}

void NotificationListModel::refrectAggregation()
{
    int prev_row = -1;
    for (const auto &cid : qAsConst(m_originalCidList)) {
        if (checkVisibility(cid)) {
            // 表示させる
            if (!aggregateQueuedPosts(cid, true)) {
                // aggregateQueuedPosts()の中では消せないので、ここで消す
                int r = m_cidList.indexOf(cid);
                if (r >= 0) {
                    beginRemoveRows(QModelIndex(), r, r);
                    m_cidList.removeAt(r);
                    endRemoveRows();
                    prev_row = r - 1;
                }
            } else {
                if (m_cidList.contains(cid)) {
                    prev_row = m_cidList.indexOf(cid);
                } else {
                    prev_row++;
                    beginInsertRows(QModelIndex(), prev_row, prev_row);
                    m_cidList.insert(prev_row, cid);
                    endInsertRows();
                }
            }
        }
    }
}

bool NotificationListModel::aggregateQueuedPosts(const QString &cid, const bool next)
{

    // 消す対象（自分より古いLike）がなかったときにcidListへの追加するのでデフォルトtrue
    bool do_add = true;
    const auto &current = m_notificationHash.value(cid);
    QString subject_cid;
    QHash<QString, QStringList> *aggregatedTo = nullptr;
    if (current.reason == "like") {
        const auto &record = AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                AtProtocolType::AppBskyFeedLike::Main>(current.record);
        subject_cid = record.subject.cid;
        aggregatedTo = &m_liked2Notification;
    } else if (current.reason == "repost") {
        const auto &record = AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                AtProtocolType::AppBskyFeedRepost::Main>(current.record);
        subject_cid = record.subject.cid;
        aggregatedTo = &m_reposted2Notification;
    } else if (current.reason == "follow") {
        subject_cid = "__follow__";
        aggregatedTo = &m_follow2Notification;
    }

    if (!subject_cid.isEmpty() && aggregatedTo != nullptr) {
        if (!aggregateReactions()) {
            if (aggregatedTo->value(subject_cid).contains(current.cid)) {
                int r = -1;
                if (!(*aggregatedTo)[subject_cid].isEmpty()) {
                    r = m_cidList.indexOf((*aggregatedTo)[subject_cid].first());
                }
                (*aggregatedTo)[subject_cid].clear();
                if (r >= 0) {
                    emit dataChanged(index(r), index(r));
                }
            }
        } else {
            for (const auto &like_cid : aggregatedTo->value(subject_cid)) {
                int r = m_cidList.indexOf(like_cid);
                if (r >= 0) {
                    if (next) {
                        // 続きの読み込みのときはm_cidListに追加しないようにする
                        do_add = false;
                    } else {
                        beginRemoveRows(QModelIndex(), r, r);
                        m_cidList.removeAt(r);
                        endRemoveRows();
                    }
                }
            }
            if (!aggregatedTo->value(subject_cid).contains(current.cid)) {
                if (next) {
                    (*aggregatedTo)[subject_cid].append(current.cid);
                } else {
                    (*aggregatedTo)[subject_cid].insert(0, current.cid);
                }
                int r = m_cidList.indexOf((*aggregatedTo)[subject_cid].first());
                if (r >= 0) {
                    emit dataChanged(index(r), index(r));
                }
            }
        }
    }
    return do_add;
}

bool NotificationListModel::aggregated(const QString &cid) const
{
    if (!aggregateReactions())
        return false;

    const auto &current = m_notificationHash.value(cid);
    QString subject_cid;
    const QHash<QString, QStringList> *aggregatedTo = nullptr;
    if (current.reason == "like") {
        const auto &record = AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                AtProtocolType::AppBskyFeedLike::Main>(current.record);
        subject_cid = record.subject.cid;
        aggregatedTo = &m_liked2Notification;
    } else if (current.reason == "repost") {
        const auto &record = AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                AtProtocolType::AppBskyFeedRepost::Main>(current.record);
        subject_cid = record.subject.cid;
        aggregatedTo = &m_reposted2Notification;
    } else if (current.reason == "follow") {
        subject_cid = "__follow__";
        aggregatedTo = &m_follow2Notification;
    }
    if (!subject_cid.isEmpty() && aggregatedTo != nullptr) {
        if (aggregatedTo->contains(subject_cid)) {
            QString first = aggregatedTo->value(subject_cid).first();
            if (cid == first) {
                // 先頭が自分の時は集約されていない
                return false;
            } else {
                return (m_cidList.contains(first));
            }
        }
    }
    return false;
}

void NotificationListModel::getPosts()
{
    if (m_cueGetPost.isEmpty()) {
        getFeedGenerators();
        return;
    }

    // getPostsは最大25個までいっきに取得できる
    QStringList uris;
    for (int i = 0; i < 25; i++) {
        if (m_cueGetPost.isEmpty())
            break;
        if (m_cueGetPost.first().contains("/app.bsky.feed.post/")) {
            uris.append(m_cueGetPost.first());
        }
        m_cueGetPost.removeFirst();
    }

    AppBskyFeedGetPosts *posts = new AppBskyFeedGetPosts(this);
    connect(posts, &AppBskyFeedGetPosts::finished, [=](bool success) {
        if (success) {
            QStringList new_cid;
            for (const auto &post : posts->postsList()) {
                //                qDebug() << post.cid << post.author.displayName;
                //                AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                //                        AtProtocolType::AppBskyFeedPost::Record>(post.record)
                //                        .text;
                m_postHash[post.cid] = post;
                new_cid.append(post.cid);
            }

            // 取得した個別のポストデータを表示用のcidリストのどの分か探して紐付ける
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
                        if ((!post_quote.embed_AppBskyEmbedRecordWithMedia_Main.record.isNull()
                             && new_cid.contains(post_quote.embed_AppBskyEmbedRecordWithMedia_Main
                                                         .record->record.cid))
                            || new_cid.contains(m_cidList.at(i))) {
                            emit dataChanged(index(i), index(i));
                        }
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
            emit errorOccured(posts->errorCode(), posts->errorMessage());
        }
        // 残ってたらもう1回
        QTimer::singleShot(10, this, &NotificationListModel::getPosts);
        posts->deleteLater();
    });
    posts->setAccount(account());
    posts->setLabelers(labelerDids());
    posts->getPosts(uris);
}

void NotificationListModel::getFeedGenerators()
{
    if (m_cueGetFeedGenerator.isEmpty()) {
        updateSeen();
        return;
    }

    // 最大25個までいっきに取得できる
    QStringList uris;
    for (int i = 0; i < 25; i++) {
        if (m_cueGetFeedGenerator.isEmpty())
            break;
        uris.append(m_cueGetFeedGenerator.first());
        m_cueGetFeedGenerator.removeFirst();
    }

    AppBskyFeedGetFeedGenerators *generators = new AppBskyFeedGetFeedGenerators(this);
    connect(generators, &AppBskyFeedGetFeedGenerators::finished, [=](bool success) {
        if (success) {
            QStringList new_cid;
            for (const auto &generator : generators->feedsList()) {
                m_feedGeneratorHash[generator.cid] = generator;
                new_cid.append(generator.cid);
            }
            // 取得した個別のポストデータを表示用のcidリストのどの分か探して紐付ける
            for (int i = 0; i < m_cidList.count(); i++) {
                if (!m_notificationHash.contains(m_cidList.at(i)))
                    continue;

                if (m_notificationHash[m_cidList.at(i)].reason == "like") {
                    emitRecordDataChanged<AtProtocolType::AppBskyFeedLike::Main>(
                            i, new_cid, m_notificationHash[m_cidList.at(i)].record);
                }
            }
        } else {
            emit errorOccured(generators->errorCode(), generators->errorMessage());
        }
        // 残ってたらもう1回
        QTimer::singleShot(10, this, &NotificationListModel::getFeedGenerators);
        generators->deleteLater();
    });
    generators->setAccount(account());
    generators->getFeedGenerators(uris);
}

void NotificationListModel::updateSeen()
{
    if (!updateSeenNotification()) {
        setRunning(false);
        return;
    }
    if (!m_hasUnread) {
        qDebug() << "All notifications are read.";
        setRunning(false);
        return;
    }

    AppBskyNotificationUpdateSeen *seen = new AppBskyNotificationUpdateSeen(this);
    connect(seen, &AppBskyNotificationUpdateSeen::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(seen->errorCode(), seen->errorMessage());
        }
        setRunning(false);
        seen->deleteLater();
    });
    seen->setAccount(account());
    seen->updateSeen(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
}

void NotificationListModel::updateMuteThread(const QStringList &cids, bool new_value)
{
    int row = -1;
    for (const auto &cid : cids) {
        row = m_cidList.indexOf(cid);
        if (row >= 0) {
            update(row, ThreadMutedRole, new_value);
        }
    }
}

QStringList NotificationListModel::getAggregatedItems(
        const AtProtocolType::AppBskyNotificationListNotifications::Notification &data,
        const NotificationListModelRoles role) const
{
    QStringList aggregated;
    for (const auto &liked_cid : getAggregatedCids(data)) {
        if (m_notificationHash.contains(liked_cid)) {
            if (role == AggregatedAvatarsRole) {
                aggregated.append(m_notificationHash.value(liked_cid).author.avatar);
            } else if (role == AggregatedDisplayNamesRole) {
                aggregated.append(m_notificationHash.value(liked_cid).author.displayName);
            } else if (role == AggregatedDidsRole) {
                aggregated.append(m_notificationHash.value(liked_cid).author.did);
            } else if (role == AggregatedHandlesRole) {
                aggregated.append(m_notificationHash.value(liked_cid).author.handle);
            } else if (role == AggregatedIndexedAtsRole) {
                aggregated.append(AtProtocolType::LexiconsTypeUnknown::formatDateTime(
                        m_notificationHash.value(liked_cid).indexedAt));
            }
        }
    }
    return aggregated;
}

QStringList NotificationListModel::getAggregatedCids(
        const AtProtocolType::AppBskyNotificationListNotifications::Notification &data) const
{
    if (data.reason == "like") {
        const auto &record = AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                AtProtocolType::AppBskyFeedLike::Main>(data.record);
        return m_liked2Notification.value(record.subject.cid);
    } else if (data.reason == "repost") {
        const auto &record = AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                AtProtocolType::AppBskyFeedRepost::Main>(data.record);
        return m_reposted2Notification.value(record.subject.cid);
    } else if (data.reason == "follow") {
        return m_follow2Notification.value("__follow__");
    } else {
        return QStringList();
    }
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
    else if (reason == "starterpack-joined")
        return true;

    return false;
}

bool NotificationListModel::runningRepost(int row) const
{
    return item(row, RunningRepostRole).toBool();
}
void NotificationListModel::setRunningRepost(int row, bool running)
{
    update(row, RunningRepostRole, running);
}
bool NotificationListModel::runningLike(int row) const
{
    return item(row, RunningLikeRole).toBool();
}
void NotificationListModel::setRunningLike(int row, bool running)
{
    update(row, RunningLikeRole, running);
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
void NotificationListModel::appendGetFeedGeneratorCue(const QVariant &record)
{
    T data = AtProtocolType::LexiconsTypeUnknown::fromQVariant<T>(record);
    if (!data.subject.cid.isEmpty() && !m_cueGetFeedGenerator.contains(data.subject.uri)) {
        m_cueGetFeedGenerator.append(data.subject.uri);
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

bool NotificationListModel::updateSeenNotification() const
{
    return m_updateSeenNotification;
}

void NotificationListModel::setUpdateSeenNotification(bool newUpdateSeenNotification)
{
    if (m_updateSeenNotification == newUpdateSeenNotification)
        return;
    m_updateSeenNotification = newUpdateSeenNotification;
    emit updateSeenNotificationChanged();
}

bool NotificationListModel::aggregateReactions() const
{
    return m_aggregateReactions;
}

void NotificationListModel::setAggregateReactions(bool newAggregateReactions)
{
    if (m_aggregateReactions == newAggregateReactions)
        return;
    m_aggregateReactions = newAggregateReactions;
    emit aggregateReactionsChanged();
    refrectAggregation();
}
