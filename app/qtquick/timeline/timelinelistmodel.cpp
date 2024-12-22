#include "timelinelistmodel.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"
#include "atprotocol/app/bsky/graph/appbskygraphmutethread.h"
#include "atprotocol/app/bsky/graph/appbskygraphunmutethread.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "operation/recordoperator.h"
#include "tools/pinnedpostcache.h"

#include <QDebug>

using AtProtocolInterface::AccountData;
using AtProtocolInterface::AppBskyFeedGetPosts;
using AtProtocolInterface::AppBskyFeedGetTimeline;
using AtProtocolInterface::AppBskyGraphMuteThread;
using AtProtocolInterface::AppBskyGraphUnmuteThread;
using namespace AtProtocolType;

TimelineListModel::TimelineListModel(QObject *parent)
    : AtpAbstractListModel { parent, true },
      m_visibleReplyToUnfollowedUsers(true),
      m_visibleRepostOfOwn(true),
      m_visibleRepostOfFollowingUsers(true),
      m_visibleRepostOfUnfollowingUsers(true),
      m_visibleRepostOfMine(true),
      m_visibleRepostByMe(true)
{
    m_toQuoteRecordRoles[HasQuoteRecordRole] =
            AtpAbstractListModel::QuoteRecordRoles::HasQuoteRecordRole;
    m_toQuoteRecordRoles[QuoteRecordIsMineRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordIsMineRole;
    m_toQuoteRecordRoles[QuoteRecordCidRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordCidRole;
    m_toQuoteRecordRoles[QuoteRecordUriRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordUriRole;
    m_toQuoteRecordRoles[QuoteRecordDisplayNameRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordDisplayNameRole;
    m_toQuoteRecordRoles[QuoteRecordHandleRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordHandleRole;
    m_toQuoteRecordRoles[QuoteRecordAvatarRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordAvatarRole;
    m_toQuoteRecordRoles[QuoteRecordRecordTextRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordRecordTextRole;
    m_toQuoteRecordRoles[QuoteRecordIndexedAtRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordIndexedAtRole;
    m_toQuoteRecordRoles[QuoteRecordEmbedImagesRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordEmbedImagesRole;
    m_toQuoteRecordRoles[QuoteRecordEmbedImagesFullRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordEmbedImagesFullRole;
    m_toQuoteRecordRoles[QuoteRecordEmbedImagesAltRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordEmbedImagesAltRole;
    m_toQuoteRecordRoles[QuoteRecordDetatchedRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordDetatchedRole;
    m_toQuoteRecordRoles[QuoteRecordBlockedRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordBlockedRole;
    m_toQuoteRecordRoles[QuoteRecordBlockedStatusRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordBlockedStatusRole;
    m_toQuoteRecordRoles[QuoteRecordHasVideoRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordHasVideoRole;
    m_toQuoteRecordRoles[QuoteRecordVideoPlaylistRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordVideoPlaylistRole;
    m_toQuoteRecordRoles[QuoteRecordVideoThumbRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordVideoThumbRole;
    m_toQuoteRecordRoles[QuoteRecordVideoAltRole] =
            AtpAbstractListModel::QuoteRecordRoles::QuoteRecordVideoAltRole;

    m_toEmbedVideoRoles[HasVideoRole] = AtpAbstractListModel::EmbedVideoRoles::HasVideoRole;
    m_toEmbedVideoRoles[VideoPlaylistRole] =
            AtpAbstractListModel::EmbedVideoRoles::VideoPlaylistRole;
    m_toEmbedVideoRoles[VideoThumbRole] = AtpAbstractListModel::EmbedVideoRoles::VideoThumbRole;
    m_toEmbedVideoRoles[VideoAltRole] = AtpAbstractListModel::EmbedVideoRoles::VideoAltRole;

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

    m_toThreadGateRoles[ThreadGateUriRole] =
            AtpAbstractListModel::ThreadGateRoles::ThreadGateUriRole;
    m_toThreadGateRoles[ThreadGateTypeRole] =
            AtpAbstractListModel::ThreadGateRoles::ThreadGateTypeRole;
    m_toThreadGateRoles[ThreadGateRulesRole] =
            AtpAbstractListModel::ThreadGateRoles::ThreadGateRulesRole;

    connect(PinnedPostCache::getInstance(), &PinnedPostCache::updated, this,
            &TimelineListModel::updatedPin);
}

TimelineListModel::~TimelineListModel()
{
    disconnect(PinnedPostCache::getInstance(), &PinnedPostCache::updated, this,
               &TimelineListModel::updatedPin);
}

int TimelineListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_cidList.count();
}

QVariant TimelineListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<TimelineListModelRoles>(role));
}

QVariant TimelineListModel::item(int row, TimelineListModelRoles role) const
{
    if (row < 0 || row >= m_cidList.count())
        return QVariant();

    const AppBskyFeedDefs::FeedViewPost &current = m_viewPostHash.value(m_cidList.at(row));

    if (role == CidRole)
        return current.post.cid;
    else if (role == UriRole)
        return current.post.uri;
    else if (role == DidRole)
        return current.post.author.did;
    else if (role == DisplayNameRole)
        return current.post.author.displayName;
    else if (role == HandleRole)
        return current.post.author.handle;
    else if (role == AvatarRole)
        return current.post.author.avatar;
    else if (role == AuthorLabelsRole)
        return getLabels(current.post.author.labels, true);
    else if (role == MutedRole)
        return current.post.author.viewer.muted;
    else if (role == RecordTextRole)
        return LexiconsTypeUnknown::copyRecordText(current.post.record);
    else if (role == RecordTextPlainRole)
        return LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(current.post.record).text;
    else if (role == RecordTextTranslationRole)
        return getTranslation(current.post.cid);
    else if (role == ReplyCountRole)
        return current.post.replyCount;
    else if (role == RepostCountRole)
        return current.post.repostCount;
    else if (role == QuoteCountRole)
        return current.post.quoteCount;
    else if (role == LikeCountRole)
        return current.post.likeCount;
    else if (role == ReplyDisabledRole)
        return current.post.viewer.replyDisabled;
    else if (role == QuoteDisabledRole)
        return current.post.viewer.embeddingDisabled;
    else if (role == IndexedAtRole)
        return LexiconsTypeUnknown::formatDateTime(current.post.indexedAt);
    else if (role == IndexedAtLongRole)
        return LexiconsTypeUnknown::formatDateTime(current.post.indexedAt, true);
    else if (role == EmbedImagesRole)
        return copyImagesFromPostView(current.post, LexiconsTypeUnknown::CopyImageType::Thumb);
    else if (role == EmbedImagesFullRole)
        return copyImagesFromPostView(current.post, LexiconsTypeUnknown::CopyImageType::FullSize);
    else if (role == EmbedImagesAltRole)
        return LexiconsTypeUnknown::copyImagesFromPostView(current.post,
                                                           LexiconsTypeUnknown::CopyImageType::Alt);
    else if (role == EmbedImagesRatioRole)
        return LexiconsTypeUnknown::copyImagesFromPostView(
                current.post, LexiconsTypeUnknown::CopyImageType::Ratio);

    else if (role == IsRepostedRole)
        return current.post.viewer.repost.contains(account().did);
    else if (role == IsLikedRole)
        return current.post.viewer.like.contains(account().did);
    else if (role == PinnedRole)
        return isPinnedPost(current.post.cid) && row == 0;
    else if (role == PinnedByMeRole)
        // return current.post.viewer.pinned;
        return PinnedPostCache::getInstance()->pinned(account().did, current.post.uri);
    else if (role == ThreadMutedRole)
        return current.post.viewer.threadMuted;
    else if (role == RepostedUriRole)
        return current.post.viewer.repost;
    else if (role == LikedUriRole)
        return current.post.viewer.like;
    else if (role == RunningRepostRole)
        return !current.post.cid.isEmpty() && (current.post.cid == m_runningRepostCid);
    else if (role == RunningLikeRole)
        return !current.post.cid.isEmpty() && (current.post.cid == m_runningLikeCid);
    else if (role == RunningdeletePostRole)
        return !current.post.cid.isEmpty() && (current.post.cid == m_runningDeletePostCid);
    else if (role == RunningPostPinningRole)
        return !current.post.cid.isEmpty() && (current.post.cid == m_runningPostPinningCid);
    else if (role == RunningOtherPrcessingRole)
        return !current.post.cid.isEmpty() && (current.post.cid == m_runningOtherProcessingCid);

    else if (role == HasQuoteRecordRole || role == QuoteRecordIsMineRole
             || role == QuoteRecordCidRole || role == QuoteRecordUriRole
             || role == QuoteRecordDisplayNameRole || role == QuoteRecordHandleRole
             || role == QuoteRecordAvatarRole || role == QuoteRecordRecordTextRole
             || role == QuoteRecordIndexedAtRole || role == QuoteRecordEmbedImagesRole
             || role == QuoteRecordEmbedImagesFullRole || role == QuoteRecordEmbedImagesAltRole
             || role == QuoteRecordDetatchedRole || role == QuoteRecordBlockedRole
             || role == QuoteRecordBlockedStatusRole || role == QuoteRecordHasVideoRole
             || role == QuoteRecordVideoPlaylistRole || role == QuoteRecordVideoThumbRole
             || role == QuoteRecordVideoAltRole)
        return getQuoteItem(
                current.post,
                m_toQuoteRecordRoles.value(
                        role, AtpAbstractListModel::QuoteRecordRoles::HasQuoteRecordRole));

    else if (m_toEmbedVideoRoles.contains(role))
        return getEmbedVideoItem(current.post, m_toEmbedVideoRoles[role]);

    else if (m_toExternalLinkRoles.contains(role))
        return getExternalLinkItem(current.post, m_toExternalLinkRoles[role]);

    else if (m_toFeedGeneratorRoles.contains(role))
        return getFeedGeneratorItem(current.post, m_toFeedGeneratorRoles[role]);

    else if (m_toListLinkRoles.contains(role))
        return getListLinkItem(current.post, m_toListLinkRoles[role]);

    else if (role == HasReplyRole) {
        if (isPinnedPost(current.post.cid) && row == 0)
            // 固定ポストは基本getPostsで取得したデータでcurrent.replyがないので表示を合わせるために非表示固定
            return false;
        else if (current.reply.parent_type == AppBskyFeedDefs::ReplyRefParentType::parent_PostView)
            return current.reply.parent_PostView.cid.length() > 0;
        else
            return false;
    } else if (role == ReplyRootCidRole) {
        if (current.reply.root_type == AppBskyFeedDefs::ReplyRefRootType::root_PostView)
            return current.reply.root_PostView.cid;
        else
            return AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                           AtProtocolType::AppBskyFeedPost::Main>(current.post.record)
                    .reply.root.cid;
    } else if (role == ReplyRootUriRole) {
        if (current.reply.root_type == AppBskyFeedDefs::ReplyRefRootType::root_PostView)
            return current.reply.root_PostView.uri;
        else
            return AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                           AtProtocolType::AppBskyFeedPost::Main>(current.post.record)
                    .reply.root.uri;
    } else if (role == ReplyParentDisplayNameRole) {
        if (current.reply.parent_type == AppBskyFeedDefs::ReplyRefParentType::parent_PostView)
            return current.reply.parent_PostView.author.displayName;
        else
            return QString();
    } else if (role == ReplyParentHandleRole)
        if (current.reply.parent_type == AppBskyFeedDefs::ReplyRefParentType::parent_PostView)
            return current.reply.parent_PostView.author.handle;
        else
            return QString();
    else if (role == IsRepostedByRole) {
        if (isPinnedPost(current.post.cid) && row == 0)
            // 固定ポストは基本getPostsで取得したデータでcurrent.replyがないので表示を合わせるために非表示固定
            return false;
        else
            return (current.reason_type
                    == AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost);
    } else if (role == RepostedByDisplayNameRole)
        return current.reason_ReasonRepost.by.displayName;
    else if (role == RepostedByHandleRole)
        return current.reason_ReasonRepost.by.handle;

    else if (role == UserFilterMatchedRole) {
        return getContentFilterMatched(current.post.author.labels, false);
    } else if (role == UserFilterMessageRole) {
        return getContentFilterMessage(current.post.author.labels, false);
    } else if (role == ContentFilterMatchedRole) {
        if (hideByMutedWords(current.post.cid, current.post.author.did)) {
            return true;
        } else {
            return getContentFilterMatched(current.post.labels, false);
        }
    } else if (role == ContentFilterMessageRole) {
        if (hideByMutedWords(current.post.cid, current.post.author.did)) {
            return tr("Post hidden by muted word");
        } else {
            return getContentFilterMessage(current.post.labels, false);
        }
    } else if (role == ContentMediaFilterMatchedRole) {
        return getContentFilterMatched(current.post.labels, true);
    } else if (role == ContentMediaFilterMessageRole) {
        return getContentFilterMessage(current.post.labels, true);
    } else if (role == QuoteFilterMatchedRole) {
        // quoteのレコードにはlangがないので保留（現状、公式と同じ）
        // QString quote_cid = getQuoteItem(current.post, QuoteRecordCidRole).toString();
        // if (!quote_cid.isEmpty() && m_mutedPosts.contains(quote_cid)) {
        //     return true;
        // } else
        if (getQuoteItem(current.post, AtpAbstractListModel::QuoteRecordRoles::HasQuoteRecordRole)
                    .toBool())
            return getQuoteFilterMatched(current.post);
        else
            return false;

    } else if (m_toThreadGateRoles.contains(role)) {
        return getThreadGateItem(current.post, m_toThreadGateRoles[role]);

    } else if (role == LabelsRole)
        return getLabels(current.post.labels);
    else if (role == LanguagesRole)
        return getLaunguages(current.post.record);
    else if (role == TagsRole)
        return QStringList(
                LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(current.post.record).tags);
    else if (role == ViaRole)
        return getVia(current.post.record);
    else if (role == CreatedAtRole)
        return LexiconsTypeUnknown::formatDateTime(
                LexiconsTypeUnknown::copyRecordCreatedAt(current.post.record), true);
    else if (role == IsArchivedRole) {
        QString created_at = LexiconsTypeUnknown::copyRecordCreatedAt(current.post.record);
        if (created_at.isEmpty() || current.post.indexedAt.isEmpty()) {
            return false;
        } else {
            QDateTime created = QDateTime::fromString(created_at, Qt::ISODateWithMs);
            QDateTime indexed = QDateTime::fromString(current.post.indexedAt, Qt::ISODateWithMs);
            return (abs(created.toMSecsSinceEpoch() - indexed.toMSecsSinceEpoch())
                    > (24 * 60 * 60 * 1000));
        }
    }

    else if (role == ThreadConnectedRole) {
        if (m_threadConnectorHash.contains(current.post.cid) && row > 0) {
            return item(row - 1, ThreadConnectorBottomRole);
        } else {
            return false;
        }
    } else if (role == ThreadConnectorTopRole) {
        if (m_threadConnectorHash.contains(current.post.cid)) {
            return m_threadConnectorHash[current.post.cid].top;
        } else {
            return false;
        }
    } else if (role == ThreadConnectorBottomRole) {
        if (m_threadConnectorHash.contains(current.post.cid)) {
            return m_threadConnectorHash[current.post.cid].bottom;
        } else {
            return false;
        }
    }

    return QVariant();
}

void TimelineListModel::update(int row, TimelineListModelRoles role, const QVariant &value)
{
    if (row < 0 || row >= m_cidList.count())
        return;

    // 外から更新しない
    // like/repostはユーザー操作を即時反映するため例外

    AppBskyFeedDefs::FeedViewPost &current = m_viewPostHash[m_cidList.at(row)];

    if (role == RepostedUriRole) {
        qDebug() << "update REPOST" << value.toString();
        current.post.viewer.repost = value.toString();
        emit dataChanged(index(row), index(row), QVector<int>() << role << IsRepostedRole);
    } else if (role == QuoteCountRole) {
        qDebug() << "update QUOTE" << value.toInt();
        current.post.quoteCount += value.toInt();
        emit dataChanged(index(row), index(row),
                         QVector<int>() << role << IsRepostedRole << QuoteCountRole);

    } else if (role == LikedUriRole) {
        qDebug() << "update LIKE" << value.toString();
        current.post.viewer.like = value.toString();
        emit dataChanged(index(row), index(row), QVector<int>() << role << IsLikedRole);

    } else if (role == RepostCountRole) {
        if (value.toBool()) {
            current.post.repostCount++;
        } else {
            current.post.repostCount--;
        }
        if (current.post.repostCount < 0) {
            current.post.repostCount = 0;
        }
        emit dataChanged(index(row), index(row), QVector<int>() << role);
    } else if (role == LikeCountRole) {
        if (value.toBool()) {
            current.post.likeCount++;
        } else {
            current.post.likeCount--;
        }
        if (current.post.likeCount < 0) {
            current.post.likeCount = 0;
        }
        emit dataChanged(index(row), index(row), QVector<int>() << role);

    } else if (role == PinnedByMeRole) {
        qDebug() << "update Pinned by me:" << value.toString();
        current.post.viewer.pinned = value.toBool();
        emit dataChanged(index(row), index(row), QVector<int>() << role << PinnedRole);
    } else if (role == ThreadMutedRole) {
        bool muted = value.toBool();
        if (current.post.viewer.threadMuted != muted) {
            current.post.viewer.threadMuted = muted;
            emit dataChanged(index(row), index(row), QVector<int>() << role);
        }
    } else if (role == RunningRepostRole) {
        if (value.toBool()) {
            m_runningRepostCid = current.post.cid;
        } else {
            m_runningRepostCid.clear();
        }
        emit dataChanged(index(row), index(row), QVector<int>() << role);
    } else if (role == RunningLikeRole) {
        if (value.toBool()) {
            m_runningLikeCid = current.post.cid;
        } else {
            m_runningLikeCid.clear();
        }
        emit dataChanged(index(row), index(row), QVector<int>() << role);
    } else if (role == RunningdeletePostRole) {
        if (value.toBool()) {
            m_runningDeletePostCid = current.post.cid;
        } else {
            m_runningDeletePostCid.clear();
        }
        emit dataChanged(index(row), index(row), QVector<int>() << role);
    } else if (role == RunningPostPinningRole) {
        if (value.toBool()) {
            m_runningPostPinningCid = current.post.cid;
        } else {
            m_runningPostPinningCid.clear();
        }
        emit dataChanged(index(row), index(row), QVector<int>() << role);
    } else if (role == RunningOtherPrcessingRole) {
        if (value.toBool()) {
            m_runningOtherProcessingCid = current.post.cid;
        } else {
            m_runningOtherProcessingCid.clear();
        }
        emit dataChanged(index(row), index(row), QVector<int>() << role);
    } else if (m_toThreadGateRoles.contains(role)) {
        updateThreadGateItem(current.post, m_toThreadGateRoles[role], value);
        emit dataChanged(index(row), index(row), QVector<int>() << role);
    }

    return;
}

int TimelineListModel::indexOf(const QString &cid) const
{
    return m_cidList.indexOf(cid);
}

QString TimelineListModel::getRecordText(const QString &cid)
{
    if (!m_cidList.contains(cid))
        return QString();
    if (!m_viewPostHash.contains(cid))
        return QString();
    return LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(m_viewPostHash[cid].post.record)
            .text;
}

QString TimelineListModel::getItemOfficialUrl(int row) const
{
    return atUriToOfficialUrl(item(row, UriRole).toString(), QStringLiteral("post"));
}

QList<int> TimelineListModel::indexsOf(const QString &cid) const
{
    int i = -1;
    QList<int> rows;
    while ((i = m_cidList.indexOf(cid, i + 1)) >= 0) {
        rows.append(i);
    }
    return rows;
}

bool TimelineListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetTimeline *timeline = new AppBskyFeedGetTimeline(this);
        connect(timeline, &AppBskyFeedGetTimeline::finished, [=](bool success) {
            if (success) {
                if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                    m_cursor = timeline->cursor();
                }
                copyFrom(timeline->feedList());
            } else {
                emit errorOccured(timeline->errorCode(), timeline->errorMessage());
            }
            QTimer::singleShot(100, this, &TimelineListModel::displayQueuedPosts);
            timeline->deleteLater();
        });
        timeline->setAccount(account());
        timeline->setLabelers(labelerDids());
        timeline->getTimeline(QString(), 0, QString());
    });
    return true;
}

bool TimelineListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetTimeline *timeline = new AppBskyFeedGetTimeline(this);
        connect(timeline, &AppBskyFeedGetTimeline::finished, [=](bool success) {
            if (success) {
                m_cursor = timeline->cursor(); // 続きの読み込みの時は必ず上書き

                copyFromNext(timeline->feedList());
            } else {
                emit errorOccured(timeline->errorCode(), timeline->errorMessage());
            }
            QTimer::singleShot(10, this, &TimelineListModel::displayQueuedPostsNext);
            timeline->deleteLater();
        });
        timeline->setAccount(account());
        timeline->setLabelers(labelerDids());
        timeline->getTimeline(QString(), 0, m_cursor);
    });
    return true;
}

bool TimelineListModel::deletePost(int row)
{
    if (row < 0 || row >= m_cidList.count())
        return false;

    if (runningdeletePost(row))
        return false;
    setRunningdeletePost(row, true);

    RecordOperator *ope = new RecordOperator(this);
    connect(ope, &RecordOperator::errorOccured, this, &TimelineListModel::errorOccured);
    connect(ope, &RecordOperator::finished, this,
            [=](bool success, const QString &uri, const QString &cid) {
                Q_UNUSED(uri)
                Q_UNUSED(cid)
                if (success) {
                    beginRemoveRows(QModelIndex(), row, row);
                    m_cidList.removeAt(row);
                    endRemoveRows();
                }
                setRunningdeletePost(row, false);
                ope->deleteLater();
            });
    ope->setAccount(account().uuid);
    ope->deletePost(item(row, UriRole).toString());

    return true;
}

bool TimelineListModel::repost(int row, bool do_count_up)
{
    if (row < 0 || row >= m_cidList.count())
        return false;

    bool current = item(row, IsRepostedRole).toBool();

    if (runningRepost(row))
        return false;
    setRunningRepost(row, true);

    const QString target_cid = item(row, CidRole).toString();
    const QString target_uri = item(row, UriRole).toString();

    RecordOperator *ope = new RecordOperator(this);
    connect(ope, &RecordOperator::errorOccured, this, &TimelineListModel::errorOccured);
    connect(ope, &RecordOperator::finished, this,
            [=](bool success, const QString &uri, const QString &cid) {
                Q_UNUSED(cid)
                const QList<int> rows = indexsOf(target_cid);
                if (success) {
                    bool first = true;
                    for (const auto &r : rows) {
                        if (first) {
                            update(row, RepostedUriRole, uri);
                            if (do_count_up) {
                                update(r, RepostCountRole, !uri.isEmpty());
                            } else if (uri.isEmpty()) {
                                // 減算のみ、加算はfirehose経由で実施
                                update(r, RepostCountRole, false);
                            }
                            first = false;
                        } else {
                            emit dataChanged(index(r), index(r),
                                             QVector<int>() << RepostedUriRole << IsRepostedRole
                                                            << RepostCountRole);
                        }
                    }
                }
                for (const auto &r : rows) {
                    setRunningRepost(r, false);
                }
                ope->deleteLater();
            });
    ope->setAccount(account().uuid);
    if (!current)
        ope->repost(target_cid, target_uri);
    else
        ope->deleteRepost(item(row, RepostedUriRole).toString());

    return true;
}

bool TimelineListModel::like(int row, bool do_count_up)
{
    if (row < 0 || row >= m_cidList.count())
        return false;

    bool current = item(row, IsLikedRole).toBool();

    if (runningLike(row))
        return false;
    setRunningLike(row, true);

    const QString target_cid = item(row, CidRole).toString();
    const QString target_uri = item(row, UriRole).toString();

    RecordOperator *ope = new RecordOperator(this);
    connect(ope, &RecordOperator::errorOccured, this, &TimelineListModel::errorOccured);
    connect(ope, &RecordOperator::finished, this,
            [=](bool success, const QString &uri, const QString &cid) {
                Q_UNUSED(cid)

                const QList<int> rows = indexsOf(target_cid);
                if (success) {
                    bool first = true;
                    for (const auto &r : rows) {
                        if (first) {
                            update(r, LikedUriRole, uri);
                            if (do_count_up) {
                                update(r, LikeCountRole, !uri.isEmpty());
                            } else if (uri.isEmpty()) {
                                // 減算のみ、加算はfirehose経由で実施
                                update(r, LikeCountRole, false);
                            }
                            first = false;
                        } else {
                            emit dataChanged(index(r), index(r),
                                             QVector<int>() << LikedUriRole << IsLikedRole
                                                            << LikeCountRole);
                        }
                    }
                }
                for (const auto &r : rows) {
                    setRunningLike(r, false);
                }
                ope->deleteLater();
            });
    ope->setAccount(account().uuid);
    if (!current)
        ope->like(target_cid, target_uri);
    else
        ope->deleteLike(item(row, LikedUriRole).toString());

    return true;
}

bool TimelineListModel::pin(int row)
{
    if (row < 0 || row >= m_cidList.count())
        return false;

    const AppBskyFeedDefs::FeedViewPost &current = m_viewPostHash.value(m_cidList.at(row));
    QString pin_uri;
    QString pin_cid;
    if (!item(row, PinnedByMeRole).toBool()) {
        pin_uri = current.post.uri;
        pin_cid = current.post.cid;
    }

    if (runningPostPinning(row))
        return false;
    setRunningPostPinning(row, true);

    RecordOperator *ope = new RecordOperator(this);
    connect(ope, &RecordOperator::errorOccured, this, &TimelineListModel::errorOccured);
    connect(ope, &RecordOperator::finished, this,
            [=](bool success, const QString &uri, const QString &cid) {
                Q_UNUSED(uri)
                Q_UNUSED(cid)
                if (success) {
                    PinnedPostCache::getInstance()->update(account().did, pin_uri);
                    // 新しい方の表示
                    m_pinnedUriCid[pin_uri] = current.post.cid;
                    emit dataChanged(index(row), index(row),
                                     QVector<int>() << PinnedRole << PinnedByMeRole);
                    // 古い方の表示の更新はPinnedPostCacheからの更新シグナルで実施
                    emit updatePin(pin_uri);
                }
                setRunningPostPinning(row, false);
                ope->deleteLater();
            });
    ope->setAccount(account().uuid);
    ope->updatePostPinning(pin_uri, pin_cid);

    return true;
}

bool TimelineListModel::muteThread(int row)
{
    if (row < 0 || row >= m_cidList.count())
        return false;

    const AppBskyFeedDefs::FeedViewPost &current = m_viewPostHash.value(m_cidList.at(row));
    const AppBskyFeedPost::Main record =
            LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(current.post.record);
    QString root_uri = record.reply.root.uri;
    if (root_uri.isEmpty() || !root_uri.startsWith("at://")) {
        // 親がないときは自身をミュートする
        root_uri = current.post.uri;
    }

    if (runningOtherPrcessing(row))
        return true;
    setRunningOtherPrcessing(row, true);

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
            setRunningOtherPrcessing(row, false);
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
            setRunningOtherPrcessing(row, false);
            thread->deleteLater();
        });
        thread->setAccount(account());
        thread->muteThread(root_uri);
    }
    return true;
}

bool TimelineListModel::detachQuote(int row)
{
    bool detached = item(row, QuoteRecordDetatchedRole).toBool();
    QString target_uri = item(row, QuoteRecordUriRole).toString();
    QString detach_uri = item(row, UriRole).toString();

    if (runningOtherPrcessing(row))
        return true;
    setRunningOtherPrcessing(row, true);

    RecordOperator *ope = new RecordOperator(this);
    connect(ope, &RecordOperator::errorOccured, this, &TimelineListModel::errorOccured);
    connect(ope, &RecordOperator::finished, this,
            [=](bool success, const QString &uri, const QString &cid) {
                Q_UNUSED(uri)
                Q_UNUSED(cid)
                if (success) {
                    // 更新後のポストを取得
                    AppBskyFeedGetPosts *post = new AppBskyFeedGetPosts(this);
                    connect(post, &AppBskyFeedGetPosts::finished, [=](bool success) {
                        if (success && !post->postsList().isEmpty()) {
                            QString new_cid = post->postsList().at(0).cid;
                            if (m_viewPostHash.contains(new_cid)) {
                                // 操作できたということは表示しているので確認するまでもないはずだけど
                                AppBskyFeedDefs::FeedViewPost feed_view_post;
                                feed_view_post.post = post->postsList().at(0);
                                m_viewPostHash[new_cid] = feed_view_post;
                                emit dataChanged(index(row), index(row));
                            }
                        }
                        setRunningOtherPrcessing(row, false);
                        post->deleteLater();
                    });
                    post->setAccount(account());
                    post->getPosts(QStringList() << detach_uri);
                } else {
                    setRunningOtherPrcessing(row, false);
                }
                ope->deleteLater();
            });
    ope->setAccount(account().uuid);
    ope->updateDetachedStatusOfQuote(detached, target_uri, detach_uri);
    return true;
}

QHash<int, QByteArray> TimelineListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[CidRole] = "cid";
    roles[UriRole] = "uri";
    roles[DidRole] = "did";
    roles[DisplayNameRole] = "displayName";
    roles[HandleRole] = "handle";
    roles[AvatarRole] = "avatar";
    roles[AuthorLabelsRole] = "authorLabels";
    roles[MutedRole] = "muted";
    roles[RecordTextRole] = "recordText";
    roles[RecordTextPlainRole] = "recordTextPlain";
    roles[RecordTextTranslationRole] = "recordTextTranslation";
    roles[ReplyCountRole] = "replyCount";
    roles[RepostCountRole] = "repostCount";
    roles[QuoteCountRole] = "quoteCount";
    roles[LikeCountRole] = "likeCount";
    roles[ReplyDisabledRole] = "replyDisabled";
    roles[QuoteDisabledRole] = "quoteDisabled";
    roles[IndexedAtRole] = "indexedAt";
    roles[IndexedAtLongRole] = "indexedAtLong";
    roles[EmbedImagesRole] = "embedImages";
    roles[EmbedImagesFullRole] = "embedImagesFull";
    roles[EmbedImagesAltRole] = "embedImagesAlt";
    roles[EmbedImagesRatioRole] = "embedImagesRatio";

    roles[IsRepostedRole] = "isReposted";
    roles[IsLikedRole] = "isLiked";
    roles[PinnedRole] = "pinned";
    roles[PinnedByMeRole] = "pinnedByMe";
    roles[ThreadMutedRole] = "threadMuted";
    roles[RepostedUriRole] = "repostedUri";
    roles[LikedUriRole] = "likedUri";
    roles[RunningRepostRole] = "runningRepost";
    roles[RunningLikeRole] = "runningLike";
    roles[RunningdeletePostRole] = "runningdeletePost";
    roles[RunningPostPinningRole] = "runningPostPinning";
    roles[RunningOtherPrcessingRole] = "runningOtherPrcessing";

    roles[HasQuoteRecordRole] = "hasQuoteRecord";
    roles[QuoteRecordIsMineRole] = "quoteRecordIsMine";
    roles[QuoteRecordCidRole] = "quoteRecordCid";
    roles[QuoteRecordUriRole] = "quoteRecordUri";
    roles[QuoteRecordDisplayNameRole] = "quoteRecordDisplayName";
    roles[QuoteRecordHandleRole] = "quoteRecordHandle";
    roles[QuoteRecordAvatarRole] = "quoteRecordAvatar";
    roles[QuoteRecordRecordTextRole] = "quoteRecordRecordText";
    roles[QuoteRecordIndexedAtRole] = "quoteRecordIndexedAt";
    roles[QuoteRecordEmbedImagesRole] = "quoteRecordEmbedImages";
    roles[QuoteRecordEmbedImagesFullRole] = "quoteRecordEmbedImagesFull";
    roles[QuoteRecordEmbedImagesAltRole] = "quoteRecordEmbedImagesAlt";
    roles[QuoteRecordDetatchedRole] = "quoteRecordDetatched";
    roles[QuoteRecordBlockedRole] = "quoteRecordBlocked";
    roles[QuoteRecordBlockedStatusRole] = "quoteRecordBlockedStatus";
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

    roles[HasReplyRole] = "hasReply";
    roles[ReplyRootCidRole] = "replyRootCid";
    roles[ReplyRootUriRole] = "replyRootUri";
    roles[ReplyParentDisplayNameRole] = "replyParentDisplayName";
    roles[ReplyParentHandleRole] = "replyParentHandle";
    roles[IsRepostedByRole] = "isRepostedBy";
    roles[RepostedByDisplayNameRole] = "repostedByDisplayName";
    roles[RepostedByHandleRole] = "repostedByHandle";

    roles[UserFilterMatchedRole] = "userFilterMatched";
    roles[UserFilterMessageRole] = "userFilterMessage";
    roles[ContentFilterMatchedRole] = "contentFilterMatched";
    roles[ContentFilterMessageRole] = "contentFilterMessage";
    roles[ContentMediaFilterMatchedRole] = "contentMediaFilterMatched";
    roles[ContentMediaFilterMessageRole] = "contentMediaFilterMessage";
    roles[QuoteFilterMatchedRole] = "quoteFilterMatched";

    roles[ThreadGateUriRole] = "threadGateUri";
    roles[ThreadGateTypeRole] = "threadGateType";
    roles[ThreadGateRulesRole] = "threadGateRules";

    roles[LabelsRole] = "labels";
    roles[LanguagesRole] = "languages";
    roles[TagsRole] = "tags";
    roles[ViaRole] = "via";
    roles[CreatedAtRole] = "createdAt";
    roles[IsArchivedRole] = "isArchived";

    roles[ThreadConnectedRole] = "threadConnected";
    roles[ThreadConnectorTopRole] = "threadConnectorTop";
    roles[ThreadConnectorBottomRole] = "threadConnectorBottom";

    return roles;
}

bool TimelineListModel::aggregateQueuedPosts(const QString &cid, const bool next)
{
    return true;
}

bool TimelineListModel::aggregated(const QString &cid) const
{
    return false;
}

void TimelineListModel::finishedDisplayingQueuedPosts()
{
    if (displayPinnedPost() && !pinnedPost().isEmpty() && !hasPinnedPost()) {
        // ピン止め対象のURLがあるけど、先頭が対象のポストじゃないときは取得にいく
        getPinnedPost();
    } else {
        setRunning(false);
    }
}

bool TimelineListModel::checkVisibility(const QString &cid)
{
    if (!m_viewPostHash.contains(cid))
        return true;

    const AppBskyFeedDefs::FeedViewPost &current = m_viewPostHash.value(cid);

    // 固定ポストの関連付け
    if (PinnedPostCache::getInstance()->pinned(account().did, current.post.uri)) {
        m_pinnedUriCid[current.post.uri] = cid;
    }

    // ミュートワードの判定
    if (cachePostsContainingMutedWords(
                current.post.cid,
                LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(current.post.record))) {
        if (current.post.author.did != account().did && !visibleContainingMutedWord()) {
            return false;
        }
    }

    for (const auto &label : current.post.author.labels) {
        if (visibilityBylabeler(label.val, false, label.src) == ConfigurableLabelStatus::Hide) {
            qDebug() << "Hide post by user's label. " << current.post.author.handle << cid;
            return false;
        }
    }
    for (const auto &label : current.post.labels) {
        if (visibilityBylabeler(label.val, true, label.src) == ConfigurableLabelStatus::Hide) {
            qDebug() << "Hide post by post's label. " << current.post.author.handle << cid;
            return false;
        }
    }
    if (!visibleReplyToUnfollowedUsers()) {
        if (current.reply.parent_type == AppBskyFeedDefs::ReplyRefParentType::parent_PostView
            && current.reply.parent_PostView.cid.length() > 0) {
            // まずreplyあり判定となる場合のみ、判断する
            if (current.post.author.did != account().did
                && !current.reply.parent_PostView.author.viewer.following.contains(account().did)) {
                qDebug() << "Hide a reply to users account do not follow. "
                         << current.post.author.handle << cid;
                return false;
            }
        }
    }
    if (!visibleRepostOfOwn()) {
        // セルフリポスト
        if (current.reason_type == AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost
            && current.reason_ReasonRepost.by.did == current.post.author.did) {
            qDebug() << "Hide reposts of user's own post." << current.post.author.handle << cid;
            return false;
        }
    }
    if (!visibleRepostOfFollowingUsers()) {
        // フォローしている人のリポスト
        if (current.reason_type == AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost
            && current.post.author.viewer.following.contains(account().did)) {
            qDebug() << "Hide reposts of posts by users you follow." << current.post.author.handle
                     << cid;
            return false;
        }
    }
    if (!visibleRepostOfUnfollowingUsers()) {
        // フォローしていない人のリポスト
        if (current.reason_type == AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost
            && !current.post.author.viewer.following.contains(account().did)
            && current.post.author.did != account().did) {
            qDebug() << "Hide reposts of posts by users you unfollow." << current.post.author.handle
                     << cid;
            return false;
        }
    }
    if (!visibleRepostOfMine()) {
        // 自分のポストのリポスト
        if (current.reason_type == AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost
            && current.post.author.did == account().did) {
            qDebug() << "Hide reposts of posts by users you unfollow." << current.post.author.handle
                     << cid;
            return false;
        }
    }
    if (!visibleRepostByMe()) {
        // 自分がしたリポスト
        if (current.reason_type == AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost
            && current.reason_ReasonRepost.by.did == account().did) {
            qDebug() << "Hide reposts by me." << current.post.author.handle << cid;
            return false;
        }
    }

    return true;
}

void TimelineListModel::copyFrom(const QList<AppBskyFeedDefs::FeedViewPost> &feed_view_post_list)
{
    QDateTime reference_time;
    int top_index = hasPinnedPost() ? 1 : 0;

    if (m_cidList.count() > top_index && m_viewPostHash.count() > 0) {
        reference_time = QDateTime::fromString(
                getReferenceTime(m_viewPostHash[m_cidList.at(top_index)]), Qt::ISODateWithMs);
    } else if (feed_view_post_list.count() > 0) {
        reference_time = QDateTime::fromString(getReferenceTime(feed_view_post_list.last()),
                                               Qt::ISODateWithMs);
    } else {
        reference_time = QDateTime::currentDateTimeUtc();
    }
    for (auto item = feed_view_post_list.crbegin(); item != feed_view_post_list.crend(); item++) {
        m_viewPostHash[item->post.cid] = *item;

        PostCueItem post;
        post.cid = item->post.cid;
        post.indexed_at = getReferenceTime(*item);
        post.reference_time = reference_time;
        post.reason_type = item->reason_type;
        m_cuePost.append(post);

        // emebed画像の取得のキューに入れる
        copyImagesFromPostViewToCue(item->post);
    }
    // embed画像を取得
    getExtendMediaFiles();
}

void TimelineListModel::copyFromNext(
        const QList<AtProtocolType::AppBskyFeedDefs::FeedViewPost> &feed_view_post_list)
{
    QDateTime reference_time = QDateTime::currentDateTimeUtc();

    for (auto item = feed_view_post_list.crbegin(); item != feed_view_post_list.crend(); item++) {
        m_viewPostHash[item->post.cid] = *item;

        PostCueItem post;
        post.cid = item->post.cid;
        post.indexed_at = getReferenceTime(*item);
        post.reference_time = reference_time;
        post.reason_type = item->reason_type;
        m_cuePost.append(post);

        // emebed画像の取得のキューに入れる
        copyImagesFromPostViewToCue(item->post);
    }
    // embed画像を取得
    getExtendMediaFiles();
}

QString
TimelineListModel::getReferenceTime(const AtProtocolType::AppBskyFeedDefs::FeedViewPost &view_post)
{
    if (view_post.reason_type == AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost) {
        return view_post.reason_ReasonRepost.indexedAt;
    } else {
        return view_post.post.indexedAt;
    }
}

void TimelineListModel::updateExtendMediaFile(const QString &parent_cid)
{
    const AppBskyFeedDefs::FeedViewPost &current = m_viewPostHash.value(parent_cid);
    // m_cidList.at(row));
    int row = m_cidList.indexOf(parent_cid);
    if (row >= 0) {
        emit dataChanged(index(row), index(row));
    }
}

bool TimelineListModel::hasPinnedPost() const
{
    if (pinnedPost().isEmpty() || m_currentPinnedPost.isEmpty())
        return false;

    const AppBskyFeedDefs::FeedViewPost &current = m_viewPostHash.value(m_currentPinnedPost);

    return (current.post.uri == pinnedPost());
}

void TimelineListModel::getPinnedPost()
{
    if (pinnedPost().isEmpty() || !pinnedPost().contains("/app.bsky.feed.post/")) {
        setRunning(false);
        return;
    }

    AppBskyFeedGetPosts *post = new AppBskyFeedGetPosts(this);
    connect(post, &AppBskyFeedGetPosts::finished, [=](bool success) {
        if (success && !post->postsList().isEmpty()) {

            QString new_cid = post->postsList().at(0).cid;

            if (!m_viewPostHash.contains(new_cid)) {
                AppBskyFeedDefs::FeedViewPost feed_view_post;
                feed_view_post.post = post->postsList().at(0);
                m_viewPostHash[new_cid] = feed_view_post;
            }

            // 前のを消す
            removePinnedPost();
            // 新しいものを追加
            bool visible = checkVisibility(new_cid);
            if (visible) {
                beginInsertRows(QModelIndex(), 0, 0);
                m_cidList.insert(0, new_cid);
                endInsertRows();
            }
            m_originalCidList.insert(0, new_cid);

            m_currentPinnedPost = new_cid;
            m_pinnedUriCid[post->postsList().at(0).uri] = new_cid;
        }
        setRunning(false);
        post->deleteLater();
    });
    post->setAccount(account());
    post->getPosts(QStringList() << pinnedPost());
}

void TimelineListModel::removePinnedPost()
{
    if (m_currentPinnedPost.isEmpty())
        return;

    if (!m_originalCidList.isEmpty() && m_originalCidList.first() == m_currentPinnedPost) {
        m_originalCidList.pop_front();
    }
    if (!m_cidList.isEmpty() && m_cidList.first() == m_currentPinnedPost) {
        beginRemoveRows(QModelIndex(), 0, 0);
        m_currentPinnedPost.clear();
        m_cidList.pop_front();
        endRemoveRows();
    }
}

void TimelineListModel::updateMuteThread(const QStringList &cids, bool new_value)
{
    int row = -1;
    for (const auto &cid : cids) {
        row = m_cidList.indexOf(cid);
        if (row >= 0) {
            update(row, ThreadMutedRole, new_value);
        }
    }
}

bool TimelineListModel::runningRepost(int row) const
{
    return item(row, RunningRepostRole).toBool();
}

void TimelineListModel::setRunningRepost(int row, bool running)
{
    update(row, RunningRepostRole, running);
}

bool TimelineListModel::runningLike(int row) const
{
    return item(row, RunningLikeRole).toBool();
}

void TimelineListModel::setRunningLike(int row, bool running)
{
    update(row, RunningLikeRole, running);
}

bool TimelineListModel::runningdeletePost(int row) const
{
    return item(row, RunningdeletePostRole).toBool();
}
void TimelineListModel::setRunningdeletePost(int row, bool running)
{
    update(row, RunningdeletePostRole, running);
}

bool TimelineListModel::runningPostPinning(int row) const
{
    return item(row, RunningPostPinningRole).toBool();
}

void TimelineListModel::setRunningPostPinning(int row, bool running)
{
    update(row, RunningPostPinningRole, running);
}

bool TimelineListModel::runningOtherPrcessing(int row) const
{
    return item(row, RunningOtherPrcessingRole).toBool();
}

void TimelineListModel::setRunningOtherPrcessing(int row, bool running)
{
    update(row, RunningOtherPrcessingRole, running);
}

bool TimelineListModel::visibleReplyToUnfollowedUsers() const
{
    return m_visibleReplyToUnfollowedUsers;
}

void TimelineListModel::setVisibleReplyToUnfollowedUsers(bool newVisibleReplyToUnfollowedUser)
{
    if (m_visibleReplyToUnfollowedUsers == newVisibleReplyToUnfollowedUser)
        return;
    m_visibleReplyToUnfollowedUsers = newVisibleReplyToUnfollowedUser;
    emit visibleReplyToUnfollowedUsersChanged();

    reflectVisibility();
}

bool TimelineListModel::visibleRepostOfOwn() const
{
    return m_visibleRepostOfOwn;
}

void TimelineListModel::setVisibleRepostOfOwn(bool newVisibleRepostOfOwn)
{
    if (m_visibleRepostOfOwn == newVisibleRepostOfOwn)
        return;
    m_visibleRepostOfOwn = newVisibleRepostOfOwn;
    emit visibleRepostOfOwnChanged();

    reflectVisibility();
}

bool TimelineListModel::visibleRepostOfFollowingUsers() const
{
    return m_visibleRepostOfFollowingUsers;
}

void TimelineListModel::setVisibleRepostOfFollowingUsers(bool newVisibleRepostOfFollowingUsers)
{
    if (m_visibleRepostOfFollowingUsers == newVisibleRepostOfFollowingUsers)
        return;
    m_visibleRepostOfFollowingUsers = newVisibleRepostOfFollowingUsers;
    emit visibleRepostOfFollowingUsersChanged();

    reflectVisibility();
}

bool TimelineListModel::visibleRepostOfUnfollowingUsers() const
{
    return m_visibleRepostOfUnfollowingUsers;
}

void TimelineListModel::setVisibleRepostOfUnfollowingUsers(bool newVisibleRepostOfUnfollowingUsers)
{
    if (m_visibleRepostOfUnfollowingUsers == newVisibleRepostOfUnfollowingUsers)
        return;
    m_visibleRepostOfUnfollowingUsers = newVisibleRepostOfUnfollowingUsers;
    emit visibleRepostOfUnfollowingUsersChanged();

    reflectVisibility();
}

bool TimelineListModel::visibleRepostOfMine() const
{
    return m_visibleRepostOfMine;
}

void TimelineListModel::setVisibleRepostOfMine(bool newVisibleRepostOfMine)
{
    if (m_visibleRepostOfMine == newVisibleRepostOfMine)
        return;
    m_visibleRepostOfMine = newVisibleRepostOfMine;
    emit visibleRepostOfMineChanged();

    reflectVisibility();
}

bool TimelineListModel::visibleRepostByMe() const
{
    return m_visibleRepostByMe;
}

void TimelineListModel::setVisibleRepostByMe(bool newVisibleRepostByMe)
{
    if (m_visibleRepostByMe == newVisibleRepostByMe)
        return;
    m_visibleRepostByMe = newVisibleRepostByMe;
    emit visibleRepostByMeChanged();

    reflectVisibility();
}

void TimelineListModel::updatedPin(const QString &did, const QString &new_uri,
                                   const QString &old_uri)
{
    int row = m_cidList.indexOf(m_pinnedUriCid.value(old_uri));
    while (row >= 0) {
        qDebug() << "updatedPin(old)" << row << old_uri << this;
        emit dataChanged(index(row), index(row), QVector<int>() << PinnedRole << PinnedByMeRole);
        row = m_cidList.indexOf(m_pinnedUriCid.value(old_uri), ++row);
    }
}
