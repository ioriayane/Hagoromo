#ifndef LEXICONS_H
#define LEXICONS_H

#include <QList>
#include <QString>
#include <QVariant>

namespace AppBskyEmbedRecord {
struct View;
}
namespace AppBskyFeedDefs {
struct ThreadViewPost;
}

namespace ComAtprotoLabelDefs {
struct Label
{
    QString src; // did
    QString uri; // uri
    QString cid; // cid
    QString val; //
    QString cts; // datetime
};
}

namespace AppBskyActorDefs {
// A reference to an actor in the network.
struct ViewerState
{
    QString blocking; // at-uri
    QString following; // at-uri
    QString followedBy; // at-uri
};
struct ProfileViewBasic
{
    QString did; // did
    QString handle; // handle
    QString displayName; //
    QString avatar; //
    ViewerState viewer;
    QList<ComAtprotoLabelDefs::Label> labels;
};
struct ProfileView
{
    QString did; // did
    QString handle; // handle
    QString displayName; //
    QString description; //
    QString avatar; //
    QString indexedAt; // datetime
    ViewerState viewer;
    QList<ComAtprotoLabelDefs::Label> labels;
};
struct ProfileViewDetailed
{
    QString did; // did
    QString handle; // handle
    QString displayName; //
    QString description; //
    QString avatar; //
    QString banner; //
    int followersCount = 0;
    int followsCount = 0;
    int postsCount = 0;
    QString indexedAt; // datetime
    ViewerState viewer;
    QList<ComAtprotoLabelDefs::Label> labels;
};
}

namespace AppBskyActorProfile {
struct Record
{
    QString displayName; //
    QString description; //
};
}

namespace AppBskyEmbedExternal {
// A representation of some externally linked content, embedded in another form of content
struct External
{
    QString uri; // uri
    QString title; //
    QString description; //
};
struct Main
{
    External external;
};
struct ViewExternal
{
    QString uri; // uri
    QString title; //
    QString description; //
    QString thumb; //
};
struct View
{
    ViewExternal external;
};
}

namespace AppBskyEmbedImages {
// A set of images embedded in some other form of content
struct Image
{
    QString alt; //
};
struct Main
{
    QList<Image> images;
};
struct ViewImage
{
    QString thumb; //
    QString fullsize; //
    QString alt; //
};
struct View
{
    QList<ViewImage> images;
};
}

namespace AppBskyEmbedRecordWithMedia {
enum class ViewMediaType : int {
    none,
    media_AppBskyEmbedImages_View,
    media_AppBskyEmbedExternal_View,
};
struct View
{
    AppBskyEmbedRecord::View *record;
    // union start : media
    ViewMediaType media_type = ViewMediaType::none;
    AppBskyEmbedImages::View media_AppBskyEmbedImages_View;
    AppBskyEmbedExternal::View media_AppBskyEmbedExternal_View;
    // union end : media
};
// A representation of a record embedded in another form of content, alongside other compatible
// embeds
struct Main
{
    // ref=app.bsky.embed.record
    // union start : media
    //     union=app.bsky.embed.images
    //     union=app.bsky.embed.external
    // union end : media
};
}

namespace AppBskyEmbedRecord {
enum class ViewRecordType : int {
    none,
    record_ViewRecord,
    record_ViewNotFound,
    record_ViewBlocked,
};
enum class ViewRecordEmbedsType : int {
    none,
    embeds_AppBskyEmbedImages_View,
    embeds_AppBskyEmbedExternal_View,
    embeds_AppBskyEmbedRecord_View,
    embeds_AppBskyEmbedRecordWithMedia_View,
};
// A representation of a record embedded in another form of content
struct Main
{
    // ref=com.atproto.repo.strongRef
};
struct ViewRecord
{
    QString uri; // at-uri
    QString cid; // cid
    AppBskyActorDefs::ProfileViewBasic author;
    QVariant value;
    QList<ComAtprotoLabelDefs::Label> labels;
    // union start : embeds
    ViewRecordEmbedsType embeds_type = ViewRecordEmbedsType::none;
    QList<AppBskyEmbedImages::View> embeds_AppBskyEmbedImages_View;
    QList<AppBskyEmbedExternal::View> embeds_AppBskyEmbedExternal_View;
    QList<AppBskyEmbedRecord::View *> embeds_AppBskyEmbedRecord_View;
    QList<AppBskyEmbedRecordWithMedia::View> embeds_AppBskyEmbedRecordWithMedia_View;
    // union end : embeds
    QString indexedAt; // datetime
};
struct ViewNotFound
{
    QString uri; // at-uri
};
struct ViewBlocked
{
    QString uri; // at-uri
};
struct View
{
    // union start : record
    ViewRecordType record_type = ViewRecordType::none;
    ViewRecord record_ViewRecord;
    ViewNotFound record_ViewNotFound;
    ViewBlocked record_ViewBlocked;
    // union end : record
};
}

namespace AppBskyFeedDefs {
enum class ThreadViewPostRepliesType : int {
    none,
    replies_ThreadViewPost,
    replies_NotFoundPost,
    replies_BlockedPost,
};
enum class ThreadViewPostParentType : int {
    none,
    parent_ThreadViewPost,
    parent_NotFoundPost,
    parent_BlockedPost,
};
enum class FeedViewPostReasonType : int {
    none,
    reason_ReasonRepost,
};
enum class PostViewEmbedType : int {
    none,
    embed_AppBskyEmbedImages_View,
    embed_AppBskyEmbedExternal_View,
    embed_AppBskyEmbedRecord_View,
    embed_AppBskyEmbedRecordWithMedia_View,
};
struct ViewerState
{
    QString repost; // at-uri
    QString like; // at-uri
};
struct PostView
{
    QString uri; // at-uri
    QString cid; // cid
    AppBskyActorDefs::ProfileViewBasic author;
    QVariant record;
    // union start : embed
    PostViewEmbedType embed_type = PostViewEmbedType::none;
    AppBskyEmbedImages::View embed_AppBskyEmbedImages_View;
    AppBskyEmbedExternal::View embed_AppBskyEmbedExternal_View;
    AppBskyEmbedRecord::View embed_AppBskyEmbedRecord_View;
    AppBskyEmbedRecordWithMedia::View embed_AppBskyEmbedRecordWithMedia_View;
    // union end : embed
    int replyCount = 0;
    int repostCount = 0;
    int likeCount = 0;
    QString indexedAt; // datetime
    ViewerState viewer;
    QList<ComAtprotoLabelDefs::Label> labels;
};
struct ReplyRef
{
    AppBskyFeedDefs::PostView root;
    AppBskyFeedDefs::PostView parent;
};
struct ReasonRepost
{
    AppBskyActorDefs::ProfileViewBasic by;
    QString indexedAt; // datetime
};
struct FeedViewPost
{
    AppBskyFeedDefs::PostView post;
    ReplyRef reply;
    // union start : reason
    FeedViewPostReasonType reason_type = FeedViewPostReasonType::none;
    ReasonRepost reason_ReasonRepost;
    // union end : reason
};
struct NotFoundPost
{
    QString uri; // at-uri
};
struct BlockedPost
{
    QString uri; // at-uri
};
struct ThreadViewPost
{
    PostView post;
    // union start : parent
    ThreadViewPostParentType parent_type = ThreadViewPostParentType::none;
    ThreadViewPost *parent_ThreadViewPost;
    NotFoundPost parent_NotFoundPost;
    BlockedPost parent_BlockedPost;
    // union end : parent
    // union start : replies
    ThreadViewPostRepliesType replies_type = ThreadViewPostRepliesType::none;
    QList<ThreadViewPost *> replies_ThreadViewPost;
    QList<NotFoundPost> replies_NotFoundPost;
    QList<BlockedPost> replies_BlockedPost;
    // union end : replies
};
}

namespace AppBskyFeedGetLikes {
struct Like
{
    QString indexedAt; // datetime
    QString createdAt; // datetime
    AppBskyActorDefs::ProfileView actor;
};
}

namespace AppBskyFeedLike {
struct Record
{
    // ref=com.atproto.repo.strongRef
    QString createdAt; // datetime
};
}

namespace AppBskyFeedPost {
struct TextSlice
{
    int start = 0;
    int end = 0;
};
struct Entity
{
    TextSlice index;
    QString type; //
    QString value; //
};
struct ReplyRef
{
    // ref=com.atproto.repo.strongRef
    // ref=com.atproto.repo.strongRef
};
struct Record
{
    QString text; //
    QList<Entity> entities;
    // ref=app.bsky.richtext.facet
    ReplyRef reply;
    // union start : embed
    //     union=app.bsky.embed.images
    //     union=app.bsky.embed.external
    //     union=app.bsky.embed.record
    //     union=app.bsky.embed.recordWithMedia
    // union end : embed
    QString createdAt; // datetime
};
}

namespace AppBskyFeedRepost {
struct Record
{
    // ref=com.atproto.repo.strongRef
    QString createdAt; // datetime
};
}

namespace AppBskyGraphBlock {
struct Record
{
    QString subject; // did
    QString createdAt; // datetime
};
}

namespace AppBskyGraphFollow {
struct Record
{
    QString subject; // did
    QString createdAt; // datetime
};
}

namespace AppBskyNotificationListNotifications {
struct Notification
{
    QString uri; // at-uri
    QString cid; // cid
    AppBskyActorDefs::ProfileView author;
    QString reason; //
    QString reasonSubject; // at-uri
    QVariant record;
    QString indexedAt; // datetime
    QList<ComAtprotoLabelDefs::Label> labels;
};
}

namespace AppBskyRichtextFacet {
enum class MainFeaturesType : int {
    none,
    features_Mention,
    features_Link,
};
struct ByteSlice
{
    int byteStart = 0;
    int byteEnd = 0;
};
struct Mention
{
    QString did; // did
};
struct Link
{
    QString uri; // uri
};
struct Main
{
    ByteSlice index;
    // union start : features
    MainFeaturesType features_type = MainFeaturesType::none;
    QList<Mention> features_Mention;
    QList<Link> features_Link;
    // union end : features
};
}

namespace ComAtprotoServerDefs {
struct InviteCodeUse
{
    QString usedBy; // did
    QString usedAt; // datetime
};
struct InviteCode
{
    QString code; //
    int available = 0;
    QString forAccount; //
    QString createdBy; //
    QString createdAt; // datetime
    QList<InviteCodeUse> uses;
};
}

namespace ComAtprotoModerationDefs {
typedef QString ReasonType;
}

namespace ComAtprotoAdminDefs {
enum class ReportViewDetailSubjectType : int {
    none,
    subject_RepoView,
    subject_RecordView,
};
enum class ActionViewDetailSubjectType : int {
    none,
    subject_RepoView,
    subject_RecordView,
};
enum class ReportViewSubjectType : int {
    none,
    subject_RepoRef,
};
enum class BlobViewDetailsType : int {
    none,
    details_ImageDetails,
    details_VideoDetails,
};
enum class ActionViewSubjectType : int {
    none,
    subject_RepoRef,
};
typedef QString ActionType;
struct RepoRef
{
    QString did; // did
};
struct ActionReversal
{
    QString reason; //
    QString createdBy; // did
    QString createdAt; // datetime
};
struct ActionView
{
    int id = 0;
    ActionType action;
    // union start : subject
    ActionViewSubjectType subject_type = ActionViewSubjectType::none;
    RepoRef subject_RepoRef;
    //     union=com.atproto.repo.strongRef
    // union end : subject
    QString reason; //
    QString createdBy; // did
    QString createdAt; // datetime
    ActionReversal reversal;
};
struct ActionViewCurrent
{
    int id = 0;
    ActionType action;
};
struct Moderation
{
    ActionViewCurrent currentAction;
};
struct RepoView
{
    QString did; // did
    QString handle; // handle
    QString email; //
    QString indexedAt; // datetime
    Moderation moderation;
    ComAtprotoServerDefs::InviteCode invitedBy;
};
struct RecordView
{
    QString uri; // at-uri
    QString cid; // cid
    QVariant value;
    QString indexedAt; // datetime
    Moderation moderation;
    RepoView repo;
};
struct ImageDetails
{
    int width = 0;
    int height = 0;
};
struct VideoDetails
{
    int width = 0;
    int height = 0;
    int length = 0;
};
struct BlobView
{
    QString cid; // cid
    QString mimeType; //
    int size = 0;
    QString createdAt; // datetime
    // union start : details
    BlobViewDetailsType details_type = BlobViewDetailsType::none;
    ImageDetails details_ImageDetails;
    VideoDetails details_VideoDetails;
    // union end : details
    Moderation moderation;
};
struct ReportView
{
    int id = 0;
    ComAtprotoModerationDefs::ReasonType reasonType;
    QString reason; //
    // union start : subject
    ReportViewSubjectType subject_type = ReportViewSubjectType::none;
    RepoRef subject_RepoRef;
    //     union=com.atproto.repo.strongRef
    // union end : subject
    QString reportedBy; // did
    QString createdAt; // datetime
};
struct ActionViewDetail
{
    int id = 0;
    ActionType action;
    // union start : subject
    ActionViewDetailSubjectType subject_type = ActionViewDetailSubjectType::none;
    RepoView subject_RepoView;
    RecordView subject_RecordView;
    // union end : subject
    QList<BlobView> subjectBlobs;
    QString reason; //
    QString createdBy; // did
    QString createdAt; // datetime
    ActionReversal reversal;
    QList<ReportView> resolvedReports;
};
struct ReportViewDetail
{
    int id = 0;
    ComAtprotoModerationDefs::ReasonType reasonType;
    QString reason; //
    // union start : subject
    ReportViewDetailSubjectType subject_type = ReportViewDetailSubjectType::none;
    RepoView subject_RepoView;
    RecordView subject_RecordView;
    // union end : subject
    QString reportedBy; // did
    QString createdAt; // datetime
    QList<ComAtprotoAdminDefs::ActionView> resolvedByActions;
};
struct ModerationDetail
{
    ActionViewCurrent currentAction;
    QList<ActionView> actions;
    QList<ReportView> reports;
};
struct RepoViewDetail
{
    QString did; // did
    QString handle; // handle
    QString email; //
    QString indexedAt; // datetime
    ModerationDetail moderation;
    QList<ComAtprotoLabelDefs::Label> labels;
    ComAtprotoServerDefs::InviteCode invitedBy;
    QList<ComAtprotoServerDefs::InviteCode> invites;
};
struct RecordViewDetail
{
    QString uri; // at-uri
    QString cid; // cid
    QVariant value;
    QList<BlobView> blobs;
    QList<ComAtprotoLabelDefs::Label> labels;
    QString indexedAt; // datetime
    ModerationDetail moderation;
    RepoView repo;
};
}

namespace ComAtprotoLabelSubscribeLabels {
struct Labels
{
    int seq = 0;
    QList<ComAtprotoLabelDefs::Label> labels;
};
struct Info
{
    QString name; //
    QString message; //
};
}

namespace ComAtprotoRepoApplyWrites {
struct Create
{
    QString collection; // nsid
    QString rkey; //
    QVariant value;
};
struct Update
{
    QString collection; // nsid
    QString rkey; //
    QVariant value;
};
struct Delete
{
    QString collection; // nsid
    QString rkey; //
};
}

namespace ComAtprotoRepoListRecords {
struct Record
{
    QString uri; // at-uri
    QString cid; // cid
    QVariant value;
};
}

namespace ComAtprotoRepoStrongRef {
// A URI with a content-hash fingerprint.
struct Main
{
    QString uri; // at-uri
    QString cid; // cid
};
}

namespace ComAtprotoServerCreateAppPassword {
struct AppPassword
{
    QString name; //
    QString password; //
    QString createdAt; // datetime
};
}

namespace ComAtprotoServerCreateInviteCodes {
struct AccountCodes
{
    QString account; //
};
}

namespace ComAtprotoServerDescribeServer {
struct Links
{
    QString privacyPolicy; //
    QString termsOfService; //
};
}

namespace ComAtprotoServerListAppPasswords {
struct AppPassword
{
    QString name; //
    QString createdAt; // datetime
};
}

namespace ComAtprotoSyncListRepos {
struct Repo
{
    QString did; // did
    QString head; // cid
};
}

namespace ComAtprotoSyncSubscribeRepos {
struct RepoOp
{
    QString action; //
    QString path; //
};
struct Commit
{
    int seq = 0;
    QString repo; // did
    QList<RepoOp> ops;
    QString time; // datetime
};
struct Handle
{
    int seq = 0;
    QString did; // did
    QString handle; // handle
    QString time; // datetime
};
struct Migrate
{
    int seq = 0;
    QString did; // did
    QString migrateTo; //
    QString time; // datetime
};
struct Tombstone
{
    int seq = 0;
    QString did; // did
    QString time; // datetime
};
struct Info
{
    QString name; //
    QString message; //
};
}

Q_DECLARE_METATYPE(AppBskyFeedPost::Record)

#endif // LEXICONS_H
