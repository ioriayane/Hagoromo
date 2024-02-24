#include "timelinelistmodel.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "recordoperator.h"

#include <QDebug>

using AtProtocolInterface::AccountData;
using AtProtocolInterface::AppBskyFeedGetTimeline;
using namespace AtProtocolType;

TimelineListModel::TimelineListModel(QObject *parent)
    : AtpAbstractListModel { parent },
      m_visibleReplyToUnfollowedUsers(true),
      m_visibleRepostOfOwn(true),
      m_visibleRepostOfFollowingUsers(true),
      m_visibleRepostOfUnfollowingUsers(true),
      m_visibleRepostOfMine(true),
      m_visibleRepostByMe(true)
{
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
    else if (role == MutedRole)
        return current.post.author.viewer.muted;
    else if (role == RecordTextRole)
        return copyRecordText(current.post.record);
    else if (role == RecordTextPlainRole)
        return LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(current.post.record).text;
    else if (role == RecordTextTranslationRole)
        return m_translations.contains(current.post.cid) ? m_translations[current.post.cid]
                                                         : QString();
    else if (role == ReplyCountRole)
        return current.post.replyCount;
    else if (role == RepostCountRole)
        return current.post.repostCount;
    else if (role == LikeCountRole)
        return current.post.likeCount;
    else if (role == ReplyDisabledRole)
        return current.post.viewer.replyDisabled;
    else if (role == IndexedAtRole)
        return formatDateTime(current.post.indexedAt);
    else if (role == IndexedAtLongRole)
        return formatDateTime(current.post.indexedAt, true);
    else if (role == EmbedImagesRole)
        return copyImagesFromPostView(current.post, LexiconsTypeUnknown::CopyImageType::Thumb);
    else if (role == EmbedImagesFullRole)
        return copyImagesFromPostView(current.post, LexiconsTypeUnknown::CopyImageType::FullSize);
    else if (role == EmbedImagesAltRole)
        return LexiconsTypeUnknown::copyImagesFromPostView(current.post,
                                                           LexiconsTypeUnknown::CopyImageType::Alt);

    else if (role == IsRepostedRole)
        return current.post.viewer.repost.contains(account().did);
    else if (role == IsLikedRole)
        return current.post.viewer.like.contains(account().did);
    else if (role == RepostedUriRole)
        return current.post.viewer.repost;
    else if (role == LikedUriRole)
        return current.post.viewer.like;

    else if (role == HasQuoteRecordRole || role == QuoteRecordCidRole || role == QuoteRecordUriRole
             || role == QuoteRecordDisplayNameRole || role == QuoteRecordHandleRole
             || role == QuoteRecordAvatarRole || role == QuoteRecordRecordTextRole
             || role == QuoteRecordIndexedAtRole || role == QuoteRecordEmbedImagesRole
             || role == QuoteRecordEmbedImagesFullRole || role == QuoteRecordEmbedImagesAltRole
             || role == QuoteRecordBlockedRole)
        return getQuoteItem(current.post, role);

    else if (m_toExternalLinkRoles.contains(role))
        return getExternalLinkItem(current.post, m_toExternalLinkRoles[role]);

    else if (m_toFeedGeneratorRoles.contains(role))
        return getFeedGeneratorItem(current.post, m_toFeedGeneratorRoles[role]);

    else if (m_toListLinkRoles.contains(role))
        return getListLinkItem(current.post, m_toListLinkRoles[role]);

    else if (role == HasReplyRole) {
        if (current.reply.parent_type == AppBskyFeedDefs::ReplyRefParentType::parent_PostView)
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
    else if (role == IsRepostedByRole)
        return (current.reason_type
                == AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost);
    else if (role == RepostedByDisplayNameRole)
        return current.reason_ReasonRepost.by.displayName;
    else if (role == RepostedByHandleRole)
        return current.reason_ReasonRepost.by.handle;

    else if (role == UserFilterMatchedRole)
        return getContentFilterMatched(current.post.author.labels, false);
    else if (role == UserFilterMessageRole)
        return getContentFilterMessage(current.post.author.labels, false);
    else if (role == ContentFilterMatchedRole)
        return getContentFilterMatched(current.post.labels, false);
    else if (role == ContentFilterMessageRole)
        return getContentFilterMessage(current.post.labels, false);
    else if (role == ContentMediaFilterMatchedRole)
        return getContentFilterMatched(current.post.labels, true);
    else if (role == ContentMediaFilterMessageRole)
        return getContentFilterMessage(current.post.labels, true);
    else if (role == QuoteFilterMatchedRole) {
        if (getQuoteItem(current.post, HasQuoteRecordRole).toBool())
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
        if (current.post.viewer.repost.isEmpty())
            current.post.repostCount--;
        else
            current.post.repostCount++;
        emit dataChanged(index(row), index(row));
    } else if (role == LikedUriRole) {
        qDebug() << "update LIKE" << value.toString();
        current.post.viewer.like = value.toString();
        if (current.post.viewer.like.isEmpty())
            current.post.likeCount--;
        else
            current.post.likeCount++;
        emit dataChanged(index(row), index(row));
    } else if (m_toThreadGateRoles.contains(role)) {
        updateThreadGateItem(current.post, m_toThreadGateRoles[role], value);
        emit dataChanged(index(row), index(row));
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
                copyFrom(timeline);
            } else {
                emit errorOccured(timeline->errorCode(), timeline->errorMessage());
            }
            QTimer::singleShot(100, this, &TimelineListModel::displayQueuedPosts);
            timeline->deleteLater();
        });
        timeline->setAccount(account());
        timeline->getTimeline();
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

                copyFromNext(timeline);
            } else {
                emit errorOccured(timeline->errorCode(), timeline->errorMessage());
            }
            QTimer::singleShot(10, this, &TimelineListModel::displayQueuedPostsNext);
            timeline->deleteLater();
        });
        timeline->setAccount(account());
        timeline->getTimeline(m_cursor);
    });
    return true;
}

bool TimelineListModel::deletePost(int row)
{
    if (row < 0 || row >= m_cidList.count())
        return false;

    if (running())
        return false;
    setRunning(true);

    RecordOperator *ope = new RecordOperator(this);
    connect(ope, &RecordOperator::errorOccured, this, &TimelineListModel::errorOccured);
    connect(ope, &RecordOperator::finished,
            [=](bool success, const QString &uri, const QString &cid) {
                Q_UNUSED(uri)
                Q_UNUSED(cid)
                if (success) {
                    beginRemoveRows(QModelIndex(), row, row);
                    m_cidList.removeAt(row);
                    endRemoveRows();
                }
                setRunning(false);
                ope->deleteLater();
            });
    ope->setAccount(account().service, account().did, account().handle, account().email,
                    account().accessJwt, account().refreshJwt);
    ope->deletePost(item(row, UriRole).toString());

    return true;
}

bool TimelineListModel::repost(int row)
{
    if (row < 0 || row >= m_cidList.count())
        return false;

    bool current = item(row, IsRepostedRole).toBool();

    if (running())
        return false;
    setRunning(true);

    RecordOperator *ope = new RecordOperator(this);
    connect(ope, &RecordOperator::errorOccured, this, &TimelineListModel::errorOccured);
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

    return true;
}

bool TimelineListModel::like(int row)
{
    if (row < 0 || row >= m_cidList.count())
        return false;

    bool current = item(row, IsLikedRole).toBool();

    if (running())
        return false;
    setRunning(true);

    RecordOperator *ope = new RecordOperator(this);
    connect(ope, &RecordOperator::errorOccured, this, &TimelineListModel::errorOccured);
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
    roles[MutedRole] = "muted";
    roles[RecordTextRole] = "recordText";
    roles[RecordTextPlainRole] = "recordTextPlain";
    roles[RecordTextTranslationRole] = "recordTextTranslation";
    roles[ReplyCountRole] = "replyCount";
    roles[RepostCountRole] = "repostCount";
    roles[LikeCountRole] = "likeCount";
    roles[ReplyDisabledRole] = "replyDisabled";
    roles[IndexedAtRole] = "indexedAt";
    roles[IndexedAtLongRole] = "indexedAtLong";
    roles[EmbedImagesRole] = "embedImages";
    roles[EmbedImagesFullRole] = "embedImagesFull";
    roles[EmbedImagesAltRole] = "embedImagesAlt";

    roles[IsRepostedRole] = "isReposted";
    roles[IsLikedRole] = "isLiked";
    roles[RepostedUriRole] = "repostedUri";
    roles[LikedUriRole] = "likedUri";

    roles[HasQuoteRecordRole] = "hasQuoteRecord";
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
    roles[QuoteRecordBlockedRole] = "quoteRecordBlocked";

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

    roles[ThreadConnectedRole] = "threadConnected";
    roles[ThreadConnectorTopRole] = "threadConnectorTop";
    roles[ThreadConnectorBottomRole] = "threadConnectorBottom";

    return roles;
}

void TimelineListModel::finishedDisplayingQueuedPosts()
{
    setRunning(false);
}

bool TimelineListModel::checkVisibility(const QString &cid)
{
    if (!m_viewPostHash.contains(cid))
        return true;

    const AppBskyFeedDefs::FeedViewPost &current = m_viewPostHash.value(cid);

    for (const auto &label : current.post.author.labels) {
        if (m_contentFilterLabels.visibility(label.val, false) == ConfigurableLabelStatus::Hide) {
            qDebug() << "Hide post by user's label. " << current.post.author.handle << cid;
            return false;
        }
    }
    for (const auto &label : current.post.labels) {
        if (m_contentFilterLabels.visibility(label.val, true) == ConfigurableLabelStatus::Hide) {
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

void TimelineListModel::copyFrom(AppBskyFeedGetTimeline *timeline)
{
    QDateTime reference_time;
    if (m_cidList.count() > 0 && m_viewPostHash.count() > 0) {
        reference_time = QDateTime::fromString(getReferenceTime(m_viewPostHash[m_cidList.at(0)]),
                                               Qt::ISODateWithMs);
    } else if (timeline->feedList()->count() > 0) {
        reference_time = QDateTime::fromString(getReferenceTime(timeline->feedList()->last()),
                                               Qt::ISODateWithMs);
    } else {
        reference_time = QDateTime::currentDateTimeUtc();
    }
    for (auto item = timeline->feedList()->crbegin(); item != timeline->feedList()->crend();
         item++) {
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

void TimelineListModel::copyFromNext(AtProtocolInterface::AppBskyFeedGetTimeline *timeline)
{
    QDateTime reference_time = QDateTime::currentDateTimeUtc();

    for (auto item = timeline->feedList()->crbegin(); item != timeline->feedList()->crend();
         item++) {
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

QVariant TimelineListModel::getQuoteItem(const AtProtocolType::AppBskyFeedDefs::PostView &post,
                                         const TimelineListModelRoles role) const
{
    bool has_record = !post.embed_AppBskyEmbedRecord_View.isNull();
    bool has_with_image = !post.embed_AppBskyEmbedRecordWithMedia_View.record.isNull();

    if (role == HasQuoteRecordRole) {
        if (has_record)
            return post.embed_type
                    == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedRecord_View
                    && post.embed_AppBskyEmbedRecord_View->record_type
                    == AppBskyEmbedRecord::ViewRecordType::record_ViewRecord;
        else if (has_with_image)
            return post.embed_type
                    == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedRecordWithMedia_View
                    && post.embed_AppBskyEmbedRecordWithMedia_View.record->record_type
                    == AppBskyEmbedRecord::ViewRecordType::record_ViewRecord;
        else
            return false;
    } else if (role == QuoteRecordCidRole) {
        if (has_record)
            return post.embed_AppBskyEmbedRecord_View->record_ViewRecord.cid;
        else if (has_with_image)
            return post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord.cid;
        else
            return QString();
    } else if (role == QuoteRecordUriRole) {
        if (has_record)
            return post.embed_AppBskyEmbedRecord_View->record_ViewRecord.uri;
        else if (has_with_image)
            return post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord.uri;
        else
            return QString();
    } else if (role == QuoteRecordDisplayNameRole) {
        if (has_record)
            return post.embed_AppBskyEmbedRecord_View->record_ViewRecord.author.displayName;
        else if (has_with_image)
            return post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord.author
                    .displayName;
        else
            return QString();
    } else if (role == QuoteRecordHandleRole) {
        if (has_record)
            return post.embed_AppBskyEmbedRecord_View->record_ViewRecord.author.handle;
        else if (has_with_image)
            return post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord.author
                    .handle;
        else
            return QString();
    } else if (role == QuoteRecordAvatarRole) {
        if (has_record)
            return post.embed_AppBskyEmbedRecord_View->record_ViewRecord.author.avatar;
        else if (has_with_image)
            return post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord.author
                    .avatar;
        else
            return QString();
    } else if (role == QuoteRecordRecordTextRole) {
        if (has_record)
            return copyRecordText(post.embed_AppBskyEmbedRecord_View->record_ViewRecord.value);
        else if (has_with_image)
            return copyRecordText(
                    post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord.value);
        else
            return QString();
    } else if (role == QuoteRecordIndexedAtRole) {
        if (has_record)
            return formatDateTime(post.embed_AppBskyEmbedRecord_View->record_ViewRecord.indexedAt);
        else if (has_with_image)
            return formatDateTime(post.embed_AppBskyEmbedRecordWithMedia_View.record
                                          ->record_ViewRecord.indexedAt);
        else
            return QString();
    } else if (role == QuoteRecordEmbedImagesRole) {
        // unionの配列で読み込んでない
        if (has_record)
            return LexiconsTypeUnknown::copyImagesFromRecord(
                    post.embed_AppBskyEmbedRecord_View->record_ViewRecord,
                    LexiconsTypeUnknown::CopyImageType::Thumb);
        else if (has_with_image)
            return LexiconsTypeUnknown::copyImagesFromRecord(
                    post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord,
                    LexiconsTypeUnknown::CopyImageType::Thumb);
        else
            return QStringList();
    } else if (role == QuoteRecordEmbedImagesFullRole) {
        // unionの配列で読み込んでない
        if (has_record)
            return LexiconsTypeUnknown::copyImagesFromRecord(
                    post.embed_AppBskyEmbedRecord_View->record_ViewRecord,
                    LexiconsTypeUnknown::CopyImageType::FullSize);
        else if (has_with_image)
            return LexiconsTypeUnknown::copyImagesFromRecord(
                    post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord,
                    LexiconsTypeUnknown::CopyImageType::FullSize);
        else
            return QStringList();
    } else if (role == QuoteRecordEmbedImagesAltRole) {
        if (has_record)
            return LexiconsTypeUnknown::copyImagesFromRecord(
                    post.embed_AppBskyEmbedRecord_View->record_ViewRecord,
                    LexiconsTypeUnknown::CopyImageType::Alt);
        else if (has_with_image)
            return LexiconsTypeUnknown::copyImagesFromRecord(
                    post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord,
                    LexiconsTypeUnknown::CopyImageType::Alt);
        else
            return QStringList();
    } else if (role == QuoteRecordBlockedRole) {
        // 引用しているポストがブロックしているユーザーのモノか
        // 付与されているラベルがHide設定の場合block表示をする
        if (has_record) {
            if (post.embed_AppBskyEmbedRecord_View->record_type
                == AppBskyEmbedRecord::ViewRecordType::record_ViewBlocked)
                return true;

            if (post.embed_AppBskyEmbedRecord_View->record_ViewRecord.author.did != account().did) {
                // 引用されているポストが他人のポストのみ判断する（自分のものの場合は隠さない）
                if (getContentFilterStatus(
                            post.embed_AppBskyEmbedRecord_View->record_ViewRecord.labels, false)
                    == ConfigurableLabelStatus::Hide)
                    return true;
                if (getContentFilterStatus(
                            post.embed_AppBskyEmbedRecord_View->record_ViewRecord.labels, true)
                    == ConfigurableLabelStatus::Hide)
                    return true;
            }
        } else if (has_with_image) {
            if (post.embed_AppBskyEmbedRecordWithMedia_View.record->record_type
                == AppBskyEmbedRecord::ViewRecordType::record_ViewBlocked)
                return true;
            if (post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord.author.did
                != account().did) {
                // 引用されているポストが他人のポストのみ判断する（自分のものの場合は隠さない）
                if (getContentFilterStatus(post.embed_AppBskyEmbedRecordWithMedia_View.record
                                                   ->record_ViewRecord.labels,
                                           false)
                    == ConfigurableLabelStatus::Hide)
                    return true;
                if (getContentFilterStatus(post.embed_AppBskyEmbedRecordWithMedia_View.record
                                                   ->record_ViewRecord.labels,
                                           true)
                    == ConfigurableLabelStatus::Hide)
                    return true;
            }
        }
        return false;
    }

    return QVariant();
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
