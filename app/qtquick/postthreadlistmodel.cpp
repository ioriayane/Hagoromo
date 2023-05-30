#include "postthreadlistmodel.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetpostthread.h"

#include <QPointer>

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
    else if (role == RecordTextTranslationRole)
        return m_translations.contains(current.cid) ? m_translations[current.cid] : QString();
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

    else if (role == IsRepostedRole)
        return current.viewer.repost.contains(account().did);
    else if (role == IsLikedRole)
        return current.viewer.like.contains(account().did);

    else if (role == HasQuoteRecordRole) {
        if (current.embed_AppBskyEmbedRecord_View.isNull()) {
            return false;
        } else {
            return current.embed_type
                    == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedRecord_View
                    && current.embed_AppBskyEmbedRecord_View->record_type
                    == AppBskyEmbedRecord::ViewRecordType::record_ViewRecord;
        }
    } else if (role == QuoteRecordCidRole) {
        if (current.embed_AppBskyEmbedRecord_View.isNull())
            return QString();
        else
            return current.embed_AppBskyEmbedRecord_View->record_ViewRecord.cid;
    } else if (role == QuoteRecordUriRole) {
        if (current.embed_AppBskyEmbedRecord_View.isNull())
            return QString();
        else
            return current.embed_AppBskyEmbedRecord_View->record_ViewRecord.uri;
    } else if (role == QuoteRecordDisplayNameRole) {
        if (current.embed_AppBskyEmbedRecord_View.isNull())
            return QString();
        else
            return current.embed_AppBskyEmbedRecord_View->record_ViewRecord.author.displayName;
    } else if (role == QuoteRecordHandleRole) {
        if (current.embed_AppBskyEmbedRecord_View.isNull())
            return QString();
        else
            return current.embed_AppBskyEmbedRecord_View->record_ViewRecord.author.handle;
    } else if (role == QuoteRecordAvatarRole) {
        if (current.embed_AppBskyEmbedRecord_View.isNull())
            return QString();
        else
            return current.embed_AppBskyEmbedRecord_View->record_ViewRecord.author.avatar;
    } else if (role == QuoteRecordRecordTextRole) {
        if (current.embed_AppBskyEmbedRecord_View.isNull())
            return QString();
        else
            return copyRecordText(current.embed_AppBskyEmbedRecord_View->record_ViewRecord.value);
    } else if (role == QuoteRecordIndexedAtRole) {
        if (current.embed_AppBskyEmbedRecord_View.isNull())
            return QString();
        else
            return formatDateTime(
                    current.embed_AppBskyEmbedRecord_View->record_ViewRecord.indexedAt);
    } else if (role == QuoteRecordEmbedImagesRole) {
        // unionの配列で読み込んでない
        if (current.embed_AppBskyEmbedRecord_View.isNull())
            return QString();
        else
            return LexiconsTypeUnknown::copyImagesFromRecord(
                    current.embed_AppBskyEmbedRecord_View->record_ViewRecord, true);
    } else if (role == QuoteRecordEmbedImagesFullRole) {
        // unionの配列で読み込んでない
        if (current.embed_AppBskyEmbedRecord_View.isNull())
            return QString();
        else
            return LexiconsTypeUnknown::copyImagesFromRecord(
                    current.embed_AppBskyEmbedRecord_View->record_ViewRecord, false);
    }

    else if (role == HasExternalLinkRole)
        return current.embed_type
                == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedExternal_View;
    else if (role == ExternalLinkUriRole)
        return current.embed_AppBskyEmbedExternal_View.external.uri;
    else if (role == ExternalLinkTitleRole)
        return current.embed_AppBskyEmbedExternal_View.external.title;
    else if (role == ExternalLinkDescriptionRole)
        return current.embed_AppBskyEmbedExternal_View.external.description;
    else if (role == ExternalLinkThumbRole)
        return current.embed_AppBskyEmbedExternal_View.external.thumb;

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

int PostThreadListModel::indexOf(const QString &cid) const
{
    return m_cidList.indexOf(cid);
}

QString PostThreadListModel::getRecordText(const QString &cid)
{
    if (!m_cidList.contains(cid))
        return QString();
    if (!m_postHash.contains(cid))
        return QString();
    return LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(m_postHash[cid].record).text;
}

void PostThreadListModel::getLatest()
{
    if (running() || postThreadUri().isEmpty())
        return;
    setRunning(true);

    QPointer<PostThreadListModel> aliving(this);

    AppBskyFeedGetPostThread *thread = new AppBskyFeedGetPostThread();
    connect(thread, &AppBskyFeedGetPostThread::finished, [=](bool success) {
        if (aliving) {
            if (success) {
                copyFrom(thread->threadViewPost());

                if (!m_cuePost.isEmpty())
                    QTimer::singleShot(100, this, &PostThreadListModel::displayQueuedPosts);
            }
            setRunning(false);
        }
        thread->deleteLater();
    });
    thread->setAccount(account());
    thread->getPostThread(postThreadUri());
}

QHash<int, QByteArray> PostThreadListModel::roleNames() const
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

    roles[IsRepostedRole] = "isReposted";
    roles[IsLikedRole] = "isLiked";

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

    roles[HasExternalLinkRole] = "hasExternalLink";
    roles[ExternalLinkUriRole] = "externalLinkUri";
    roles[ExternalLinkTitleRole] = "externalLinkTitle";
    roles[ExternalLinkDescriptionRole] = "externalLinkDescription";
    roles[ExternalLinkThumbRole] = "externalLinkThumb";

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

    QDateTime reference_time = QDateTime::currentDateTimeUtc();

    if (thread_view_post->parent_type
        == AppBskyFeedDefs::ThreadViewPostParentType::parent_ThreadViewPost) {
        copyFrom(thread_view_post->parent_ThreadViewPost.get());
    }

    PostCueItem post;
    post.cid = thread_view_post->post.cid;
    post.indexed_at = thread_view_post->post.indexedAt;
    post.reference_time = reference_time;
    m_cuePost.insert(0, post);

    m_postHash[thread_view_post->post.cid] = thread_view_post->post;

    // TODO
    // replies側の表示
    // こっちは枝分かれする場合があるので表示方法を検討すること
    // そもそも時系列が逆順になるので基準になっているpostを強調するなど工夫が必要
    // あと、repliesの1階層目しか表示できていない！
    for (const auto &view_post : thread_view_post->replies_ThreadViewPost) {
        m_postHash[view_post->post.cid] = view_post->post;

        PostCueItem post;
        post.cid = view_post->post.cid;
        post.indexed_at = view_post->post.indexedAt;
        post.reference_time = reference_time;
        m_cuePost.insert(0, post);
    }
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
