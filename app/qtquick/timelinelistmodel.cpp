#include "timelinelistmodel.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QDebug>

using AtProtocolInterface::AccountData;
using AtProtocolInterface::AppBskyFeedGetTimeline;
using namespace AtProtocolType;

TimelineListModel::TimelineListModel(QObject *parent) : AtpAbstractListModel { parent }
{
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
    else if (role == RecordTextRole)
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
    else if (role == IndexedAtRole)
        return formatDateTime(current.post.indexedAt);
    else if (role == EmbedImagesRole)
        return LexiconsTypeUnknown::copyImagesFromPostView(current.post, true);
    else if (role == EmbedImagesFullRole)
        return LexiconsTypeUnknown::copyImagesFromPostView(current.post, false);

    else if (role == HasQuoteRecordRole)
        return current.post.embed_type
                == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedRecord_View
                && current.post.embed_AppBskyEmbedRecord_View.record_type
                == AppBskyEmbedRecord::ViewRecordType::record_ViewRecord;
    else if (role == QuoteRecordCidRole)
        return current.post.embed_AppBskyEmbedRecord_View.record_ViewRecord.cid;
    else if (role == QuoteRecordUriRole)
        return current.post.embed_AppBskyEmbedRecord_View.record_ViewRecord.uri;
    else if (role == QuoteRecordDisplayNameRole)
        return current.post.embed_AppBskyEmbedRecord_View.record_ViewRecord.author.displayName;
    else if (role == QuoteRecordHandleRole)
        return current.post.embed_AppBskyEmbedRecord_View.record_ViewRecord.author.handle;
    else if (role == QuoteRecordAvatarRole)
        return current.post.embed_AppBskyEmbedRecord_View.record_ViewRecord.author.avatar;
    else if (role == QuoteRecordRecordTextRole)
        return LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(
                       current.post.embed_AppBskyEmbedRecord_View.record_ViewRecord.value)
                .text;
    else if (role == QuoteRecordIndexedAtRole)
        return formatDateTime(
                current.post.embed_AppBskyEmbedRecord_View.record_ViewRecord.indexedAt);
    else if (role == QuoteRecordEmbedImagesRole)
        // unionの配列で読み込んでない
        return LexiconsTypeUnknown::copyImagesFromRecord(
                current.post.embed_AppBskyEmbedRecord_View.record_ViewRecord, true);
    else if (role == QuoteRecordEmbedImagesFullRole)
        // unionの配列で読み込んでない
        return LexiconsTypeUnknown::copyImagesFromRecord(
                current.post.embed_AppBskyEmbedRecord_View.record_ViewRecord, false);

    else if (role == HasReplyRole)
        return current.reply.parent.cid.length() > 0;
    else if (role == ReplyRootCidRole)
        return current.reply.root.cid;
    else if (role == ReplyRootUriRole)
        return current.reply.root.uri;
    else if (role == ReplyParentDisplayNameRole)
        return current.reply.parent.author.displayName;
    else if (role == ReplyParentHandleRole)
        return current.reply.parent.author.handle;
    else if (role == IsRepostedByRole)
        return (current.reason_type
                == AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost);
    else if (role == RepostedByDisplayNameRole)
        return current.reason_ReasonRepost.by.displayName;
    else if (role == RepostedByHandleRole)
        return current.reason_ReasonRepost.by.handle;

    return QVariant();
}

void TimelineListModel::update(int row, TimelineListModelRoles role, const QVariant &value)
{
    Q_UNUSED(role)
    Q_UNUSED(value)

    if (row < 0 || row >= m_cidList.count())
        return;

    // 外から更新しない

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

void TimelineListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    AppBskyFeedGetTimeline *timeline = new AppBskyFeedGetTimeline();
    connect(timeline, &AppBskyFeedGetTimeline::finished, [=](bool success) {
        if (success) {
            copyFrom(timeline);
        }
        setRunning(false);
        timeline->deleteLater();
    });
    timeline->setAccount(account());
    timeline->getTimeline();
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
    roles[RecordTextRole] = "recordText";
    roles[RecordTextTranslationRole] = "recordTextTranslation";
    roles[ReplyCountRole] = "replyCount";
    roles[RepostCountRole] = "repostCount";
    roles[LikeCountRole] = "likeCount";
    roles[IndexedAtRole] = "indexedAt";
    roles[EmbedImagesRole] = "embedImages";
    roles[EmbedImagesFullRole] = "embedImagesFull";

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

    roles[HasReplyRole] = "hasReply";
    roles[ReplyRootCidRole] = "replyRootCid";
    roles[ReplyRootUriRole] = "replyRootUri";
    roles[ReplyParentDisplayNameRole] = "replyParentDisplayName";
    roles[ReplyParentHandleRole] = "replyParentHandle";
    roles[IsRepostedByRole] = "isRepostedBy";
    roles[RepostedByDisplayNameRole] = "repostedByDisplayName";
    roles[RepostedByHandleRole] = "repostedByHandle";

    return roles;
}

void TimelineListModel::copyFrom(AppBskyFeedGetTimeline *timeline)
{
    QDateTime reference_time;
    if (m_cidList.count() > 0 && m_viewPostHash.count() > 0) {
        reference_time = QDateTime::fromString(m_viewPostHash[m_cidList.at(0)].post.indexedAt,
                                               Qt::ISODateWithMs);
    } else if (timeline->feedList()->count() > 0) {
        reference_time = QDateTime::fromString(timeline->feedList()->last().post.indexedAt,
                                               Qt::ISODateWithMs);
    } else {
        reference_time = QDateTime::currentDateTimeUtc();
    }
    for (auto item = timeline->feedList()->crbegin(); item != timeline->feedList()->crend();
         item++) {
        m_viewPostHash[item->post.cid] = *item;

        if (m_cidList.contains(item->post.cid)) {
            if (item->reason_type == AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost
                && (QDateTime::fromString(item->post.indexedAt, Qt::ISODateWithMs)
                    > reference_time)) {
                // repostのときはいったん消す（上に持っていく）
                int r = m_cidList.indexOf(item->post.cid);
                if (r > 0) {
                    beginMoveRows(QModelIndex(), r, r, QModelIndex(), 0);
                    m_cidList.move(r, 0);
                    endMoveRows();
                }
            } else {
                // リストは更新しないでデータのみ入れ替える
                // リプライ数とかだけ更新をUIに通知
                // （取得できた範囲でしか更新できないのだけど・・・）
                int pos = m_cidList.indexOf(item->post.cid);
                emit dataChanged(index(pos), index(pos),
                                 QVector<int>()
                                         << ReplyCountRole << RepostCountRole << LikeCountRole);
            }
        } else {
            beginInsertRows(QModelIndex(), 0, 0);
            m_cidList.insert(0, item->post.cid);
            endInsertRows();
        }
    }
}
