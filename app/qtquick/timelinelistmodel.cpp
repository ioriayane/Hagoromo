#include "timelinelistmodel.h"
#include "../atprotocol/lexicons_func_unknown.h"

#include <QDebug>

using AtProtocolInterface::AccountData;
using AtProtocolInterface::AppBskyFeedGetTimeline;
using namespace AtProtocolType;

TimelineListModel::TimelineListModel(AtpAbstractListModel *parent) : AtpAbstractListModel { parent }
{
    connect(&m_timeline, &AppBskyFeedGetTimeline::finished, [=](bool success) {
        if (success) {
            QDateTime reference_time;
            if (m_cidList.count() > 0 && m_viewPostHash.count() > 0) {
                reference_time = QDateTime::fromString(
                        m_viewPostHash[m_cidList.at(0)].post.indexedAt, Qt::ISODateWithMs);
            } else if (m_timeline.feedList()->count() > 0) {
                reference_time = QDateTime::fromString(m_timeline.feedList()->last().post.indexedAt,
                                                       Qt::ISODateWithMs);
            } else {
                reference_time = QDateTime::currentDateTimeUtc();
            }
            for (auto item = m_timeline.feedList()->crbegin();
                 item != m_timeline.feedList()->crend(); item++) {
                m_viewPostHash[item->post.cid] = *item;

                if (m_cidList.contains(item->post.cid)) {
                    if (item->reason_type
                                == AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost
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
                                         QVector<int>() << ReplyCountRole << RepostCountRole
                                                        << LikeCountRole);
                    }
                } else {
                    beginInsertRows(QModelIndex(), 0, 0);
                    m_cidList.insert(0, item->post.cid);
                    endInsertRows();
                }
            }
        }
        setRunning(false);
    });
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
    else if (role == DisplayNameRole)
        return current.post.author.displayName;
    else if (role == HandleRole)
        return current.post.author.handle;
    else if (role == AvatarRole)
        return current.post.author.avatar;
    else if (role == RecordTextRole)
        return LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Record>(current.post.record).text;
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

    else if (role == HasChildRecordRole)
        return current.post.embed_type
                == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedRecord_View
                && current.post.embed_AppBskyEmbedRecord_View.record_type
                == AppBskyEmbedRecord::ViewRecordType::record_ViewRecord;
    else if (role == ChildRecordDisplayNameRole)
        return current.post.embed_AppBskyEmbedRecord_View.record_ViewRecord.author.displayName;
    else if (role == ChildRecordHandleRole)
        return current.post.embed_AppBskyEmbedRecord_View.record_ViewRecord.author.handle;
    else if (role == ChildRecordAvatarRole)
        return current.post.embed_AppBskyEmbedRecord_View.record_ViewRecord.author.avatar;
    else if (role == ChildRecordRecordTextRole)
        return LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Record>(
                       current.post.embed_AppBskyEmbedRecord_View.record_ViewRecord.value)
                .text;
    else if (role == ChildRecordIndexedAtRole)
        return formatDateTime(
                current.post.embed_AppBskyEmbedRecord_View.record_ViewRecord.indexedAt);
    else if (role == ChildRecordEmbedImagesRole)
        // unionの配列で読み込んでない
        return LexiconsTypeUnknown::copyImagesFromRecord(
                current.post.embed_AppBskyEmbedRecord_View.record_ViewRecord, true);
    else if (role == ChildRecordEmbedImagesFullRole)
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

void TimelineListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    m_timeline.setAccount(account());
    m_timeline.getTimeline();
}

QHash<int, QByteArray> TimelineListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[CidRole] = "cid";
    roles[UriRole] = "uri";
    roles[DisplayNameRole] = "displayName";
    roles[HandleRole] = "handle";
    roles[AvatarRole] = "avatar";
    roles[RecordTextRole] = "recordText";
    roles[ReplyCountRole] = "replyCount";
    roles[RepostCountRole] = "repostCount";
    roles[LikeCountRole] = "likeCount";
    roles[IndexedAtRole] = "indexedAt";
    roles[EmbedImagesRole] = "embedImages";
    roles[EmbedImagesFullRole] = "embedImagesFull";

    roles[HasChildRecordRole] = "hasChildRecord";
    roles[ChildRecordDisplayNameRole] = "childRecordDisplayName";
    roles[ChildRecordHandleRole] = "childRecordHandle";
    roles[ChildRecordAvatarRole] = "childRecordAvatar";
    roles[ChildRecordRecordTextRole] = "childRecordRecordText";
    roles[ChildRecordIndexedAtRole] = "childRecordIndexedAt";
    roles[ChildRecordEmbedImagesRole] = "childRecordEmbedImages";
    roles[ChildRecordEmbedImagesFullRole] = "childRecordEmbedImagesFull";

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
