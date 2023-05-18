#include "postthreadlistmodel.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetpostthread.h"

using AtProtocolInterface::AppBskyFeedGetPostThread;
using namespace AtProtocolType;

PostThreadListModel::PostThreadListModel(QObject *parent) : AtpAbstractListModel { parent } { }

int PostThreadListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_cidList.count();
}

QVariant PostThreadListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<PostThreadListModelRoles>(role));
}

QVariant PostThreadListModel::item(int row, PostThreadListModelRoles role) const
{
    if (row < 0 || row >= m_cidList.count())
        return QVariant();

    const AppBskyFeedDefs::PostView &current = m_postHash.value(m_cidList.at(row));

    if (role == CidRole)
        return current.cid;
    else if (role == UriRole)
        return current.uri;
    else if (role == DisplayNameRole)
        return current.author.displayName;
    else if (role == HandleRole)
        return current.author.handle;
    else if (role == AvatarRole)
        return current.author.avatar;
    else if (role == RecordTextRole)
        return LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(current.record).text;
    else if (role == ReplyCountRole)
        return current.replyCount;
    else if (role == RepostCountRole)
        return current.repostCount;
    else if (role == LikeCountRole)
        return current.likeCount;
    else if (role == IndexedAtRole)
        return formatDateTime(current.indexedAt);
    else if (role == EmbedImagesRole)
        return LexiconsTypeUnknown::copyImagesFromPostView(current, true);
    else if (role == EmbedImagesFullRole)
        return LexiconsTypeUnknown::copyImagesFromPostView(current, false);

    else if (role == HasQuoteRecordRole)
        return current.embed_type
                == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedRecord_View
                && current.embed_AppBskyEmbedRecord_View.record_type
                == AppBskyEmbedRecord::ViewRecordType::record_ViewRecord;
    else if (role == QuoteRecordCidRole)
        return current.embed_AppBskyEmbedRecord_View.record_ViewRecord.cid;
    else if (role == QuoteRecordUriRole)
        return current.embed_AppBskyEmbedRecord_View.record_ViewRecord.uri;
    else if (role == QuoteRecordDisplayNameRole)
        return current.embed_AppBskyEmbedRecord_View.record_ViewRecord.author.displayName;
    else if (role == QuoteRecordHandleRole)
        return current.embed_AppBskyEmbedRecord_View.record_ViewRecord.author.handle;
    else if (role == QuoteRecordAvatarRole)
        return current.embed_AppBskyEmbedRecord_View.record_ViewRecord.author.avatar;
    else if (role == QuoteRecordRecordTextRole)
        return LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(
                       current.embed_AppBskyEmbedRecord_View.record_ViewRecord.value)
                .text;
    else if (role == QuoteRecordIndexedAtRole)
        return formatDateTime(current.embed_AppBskyEmbedRecord_View.record_ViewRecord.indexedAt);
    else if (role == QuoteRecordEmbedImagesRole)
        // unionの配列で読み込んでない
        return LexiconsTypeUnknown::copyImagesFromRecord(
                current.embed_AppBskyEmbedRecord_View.record_ViewRecord, true);
    else if (role == QuoteRecordEmbedImagesFullRole)
        // unionの配列で読み込んでない
        return LexiconsTypeUnknown::copyImagesFromRecord(
                current.embed_AppBskyEmbedRecord_View.record_ViewRecord, false);

    else if (role == HasReplyRole)
        return false;
    else if (role == ReplyRootCidRole)
        return QString();
    else if (role == ReplyRootUriRole)
        return QString();
    else if (role == ReplyParentDisplayNameRole)
        return QString();
    else if (role == ReplyParentHandleRole)
        return QString();
    else if (role == IsRepostedByRole)
        return false;
    else if (role == RepostedByDisplayNameRole)
        return QString();
    else if (role == RepostedByHandleRole)
        return QString();

    return QVariant();
}

void PostThreadListModel::getLatest()
{
    if (running() || postThreadUri().isEmpty())
        return;
    setRunning(true);

    AppBskyFeedGetPostThread *thread = new AppBskyFeedGetPostThread();
    connect(thread, &AppBskyFeedGetPostThread::finished, [=](bool success) {
        if (success) {
            copyFrom(thread->threadViewPost());
        }

        thread->deleteLater();
        setRunning(false);
    });
    thread->setAccount(account());
    thread->getPostThread(postThreadUri());
}

QHash<int, QByteArray> PostThreadListModel::roleNames() const
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

void PostThreadListModel::copyFrom(const AppBskyFeedDefs::ThreadViewPost *thread_view_post)
{
    if (thread_view_post == nullptr)
        return;

    if (thread_view_post->parent_type
        == AppBskyFeedDefs::ThreadViewPostParentType::parent_ThreadViewPost) {
        copyFrom(thread_view_post->parent_ThreadViewPost);
    }

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_cidList.append(thread_view_post->post.cid);
    m_postHash[thread_view_post->post.cid] = thread_view_post->post;
    endInsertRows();

    // TODO
    // replies側の表示
    // こっちは枝分かれする場合があるので表示方法を検討すること
    // そもそも時系列が逆順になるので基準になっているpostを強調するなど工夫が必要
}

QString PostThreadListModel::postThreadUri() const
{
    return m_postThreadUri;
}

void PostThreadListModel::setPostThreadUri(const QString &newPostThreadUri)
{
    if (m_postThreadUri == newPostThreadUri)
        return;
    m_postThreadUri = newPostThreadUri;
    emit postThreadUriChanged();
}
