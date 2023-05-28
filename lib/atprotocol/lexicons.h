// This file is generated by "defs2struct.py".
// Please do not edit.

#ifndef LEXICONS_H
#define LEXICONS_H

#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QVariant>

namespace AtProtocolType {
namespace AppBskyActorDefs {
struct ProfileView;
}
namespace AppBskyEmbedRecord {
struct Main;
struct View;
}
namespace AppBskyFeedDefs {
struct ThreadViewPost;
}
namespace AppBskyRichtextFacet {
struct Main;
}


// app.bsky.graph.defs
namespace AppBskyGraphDefs {
typedef QString ListPurpose;
struct ListViewerState
{
};
struct ListViewBasic
{
    QString uri; // at-uri
    QString name; //
    ListPurpose purpose;
    QString avatar; //
    ListViewerState viewer;
    QString indexedAt; // datetime
};
struct ListView
{
    QString uri; // at-uri
    QSharedPointer<AppBskyActorDefs::ProfileView> creator;
    QString name; //
    ListPurpose purpose;
    QString description; //
    QList<QSharedPointer<AppBskyRichtextFacet::Main>> descriptionFacets;
    QString avatar; //
    ListViewerState viewer;
    QString indexedAt; // datetime
};
struct ListItemView
{
    QSharedPointer<AppBskyActorDefs::ProfileView> subject;
};
}

// com.atproto.label.defs
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

// app.bsky.actor.defs
namespace AppBskyActorDefs {
// A reference to an actor in the network.
struct ViewerState
{
    AppBskyGraphDefs::ListViewBasic mutedByList;
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
struct AdultContentPref
{
};
struct ContentLabelPref
{
    QString label; //
    QString visibility; //
};
struct SavedFeedsPref
{
};
}

// app.bsky.actor.profile
namespace AppBskyActorProfile {
struct Main
{
    QString displayName; //
    QString description; //
};
}

// app.bsky.embed.external
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

// app.bsky.embed.images
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

// com.atproto.repo.strongRef
namespace ComAtprotoRepoStrongRef {
struct Main
{
    QString uri; // at-uri
    QString cid; // cid
};
// A URI with a content-hash fingerprint.
}

// app.bsky.embed.recordWithMedia
namespace AppBskyEmbedRecordWithMedia {
enum class MainMediaType : int {
    none,
    media_AppBskyEmbedImages_Main,
    media_AppBskyEmbedExternal_Main,
};
enum class ViewMediaType : int {
    none,
    media_AppBskyEmbedImages_View,
    media_AppBskyEmbedExternal_View,
};
struct View
{
    QSharedPointer<AppBskyEmbedRecord::View> record;
    // union start : media
    ViewMediaType media_type = ViewMediaType::none;
    AppBskyEmbedImages::View media_AppBskyEmbedImages_View;
    AppBskyEmbedExternal::View media_AppBskyEmbedExternal_View;
    // union end : media
};
// A representation of a record embedded in another form of content, alongside other compatible embeds
struct Main
{
    QSharedPointer<AppBskyEmbedRecord::Main> record;
    // union start : media
    MainMediaType media_type = MainMediaType::none;
    AppBskyEmbedImages::Main media_AppBskyEmbedImages_Main;
    AppBskyEmbedExternal::Main media_AppBskyEmbedExternal_Main;
    // union end : media
};
}

// app.bsky.richtext.facet
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

// app.bsky.feed.defs
namespace AppBskyFeedDefs {
enum class SkeletonFeedPostReasonType : int {
    none,
    reason_SkeletonReasonRepost,
};
enum class ThreadViewPostParentType : int {
    none,
    parent_ThreadViewPost,
    parent_NotFoundPost,
    parent_BlockedPost,
};
enum class ThreadViewPostRepliesType : int {
    none,
    replies_ThreadViewPost,
    replies_NotFoundPost,
    replies_BlockedPost,
};
enum class FeedViewPostReasonType : int {
    none,
    reason_ReasonRepost,
};
enum class ReplyRefRootType : int {
    none,
    root_PostView,
    root_NotFoundPost,
    root_BlockedPost,
};
enum class ReplyRefParentType : int {
    none,
    parent_PostView,
    parent_NotFoundPost,
    parent_BlockedPost,
};
enum class PostViewEmbedType : int {
    none,
    embed_AppBskyEmbedImages_View,
    embed_AppBskyEmbedExternal_View,
    embed_AppBskyEmbedRecord_View,
    embed_AppBskyEmbedRecordWithMedia_View,
};
struct GeneratorViewerState
{
    QString like; // at-uri
};
struct GeneratorView
{
    QString uri; // at-uri
    QString cid; // cid
    QString did; // did
    AppBskyActorDefs::ProfileView creator;
    QString displayName; //
    QString description; //
    QList<AppBskyRichtextFacet::Main> descriptionFacets;
    QString avatar; //
    int likeCount = 0;
    GeneratorViewerState viewer;
    QString indexedAt; // datetime
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
    QSharedPointer<AppBskyEmbedRecord::View> embed_AppBskyEmbedRecord_View;
    AppBskyEmbedRecordWithMedia::View embed_AppBskyEmbedRecordWithMedia_View;
    // union end : embed
    int replyCount = 0;
    int repostCount = 0;
    int likeCount = 0;
    QString indexedAt; // datetime
    ViewerState viewer;
    QList<ComAtprotoLabelDefs::Label> labels;
};
struct NotFoundPost
{
    QString uri; // at-uri
};
struct BlockedPost
{
    QString uri; // at-uri
};
struct ReplyRef
{
    // union start : root
    ReplyRefRootType root_type = ReplyRefRootType::none;
    PostView root_PostView;
    NotFoundPost root_NotFoundPost;
    BlockedPost root_BlockedPost;
    // union end : root
    // union start : parent
    ReplyRefParentType parent_type = ReplyRefParentType::none;
    PostView parent_PostView;
    NotFoundPost parent_NotFoundPost;
    BlockedPost parent_BlockedPost;
    // union end : parent
};
struct ReasonRepost
{
    AppBskyActorDefs::ProfileViewBasic by;
    QString indexedAt; // datetime
};
struct FeedViewPost
{
    PostView post;
    ReplyRef reply;
    // union start : reason
    FeedViewPostReasonType reason_type = FeedViewPostReasonType::none;
    ReasonRepost reason_ReasonRepost;
    // union end : reason
};
struct ThreadViewPost
{
    PostView post;
    // union start : parent
    ThreadViewPostParentType parent_type = ThreadViewPostParentType::none;
    QSharedPointer<ThreadViewPost> parent_ThreadViewPost;
    NotFoundPost parent_NotFoundPost;
    BlockedPost parent_BlockedPost;
    // union end : parent
    // union start : replies
    ThreadViewPostRepliesType replies_type = ThreadViewPostRepliesType::none;
    QList<QSharedPointer<ThreadViewPost>> replies_ThreadViewPost;
    QList<NotFoundPost> replies_NotFoundPost;
    QList<BlockedPost> replies_BlockedPost;
    // union end : replies
};
struct SkeletonReasonRepost
{
    QString repost; // at-uri
};
struct SkeletonFeedPost
{
    QString post; // at-uri
    // union start : reason
    SkeletonFeedPostReasonType reason_type = SkeletonFeedPostReasonType::none;
    SkeletonReasonRepost reason_SkeletonReasonRepost;
    // union end : reason
};
}

// app.bsky.embed.record
namespace AppBskyEmbedRecord {
enum class ViewRecordType : int {
    none,
    record_ViewRecord,
    record_ViewNotFound,
    record_ViewBlocked,
    record_AppBskyFeedDefs_GeneratorView,
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
    ComAtprotoRepoStrongRef::Main record;
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
    QList<QSharedPointer<AppBskyEmbedRecord::View>> embeds_AppBskyEmbedRecord_View;
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
    AppBskyFeedDefs::GeneratorView record_AppBskyFeedDefs_GeneratorView;
    // union end : record
};
}

// app.bsky.feed.describeFeedGenerator
namespace AppBskyFeedDescribeFeedGenerator {
struct Feed
{
    QString uri; // at-uri
};
struct Links
{
    QString privacyPolicy; //
    QString termsOfService; //
};
}

// app.bsky.feed.generator
namespace AppBskyFeedGenerator {
struct Main
{
    QString did; // did
    QString displayName; //
    QString description; //
    QList<AppBskyRichtextFacet::Main> descriptionFacets;
    QString createdAt; // datetime
};
}

// app.bsky.feed.getLikes
namespace AppBskyFeedGetLikes {
struct Like
{
    QString indexedAt; // datetime
    QString createdAt; // datetime
    AppBskyActorDefs::ProfileView actor;
};
}

// app.bsky.feed.like
namespace AppBskyFeedLike {
struct Main
{
    ComAtprotoRepoStrongRef::Main subject;
    QString createdAt; // datetime
};
}

// app.bsky.feed.post
namespace AppBskyFeedPost {
enum class MainEmbedType : int {
    none,
    embed_AppBskyEmbedImages_Main,
    embed_AppBskyEmbedExternal_Main,
    embed_AppBskyEmbedRecord_Main,
    embed_AppBskyEmbedRecordWithMedia_Main,
};
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
    ComAtprotoRepoStrongRef::Main root;
    ComAtprotoRepoStrongRef::Main parent;
};
struct Main
{
    QString text; //
    QList<Entity> entities;
    QList<AppBskyRichtextFacet::Main> facets;
    ReplyRef reply;
    // union start : embed
    MainEmbedType embed_type = MainEmbedType::none;
    AppBskyEmbedImages::Main embed_AppBskyEmbedImages_Main;
    AppBskyEmbedExternal::Main embed_AppBskyEmbedExternal_Main;
    AppBskyEmbedRecord::Main embed_AppBskyEmbedRecord_Main;
    AppBskyEmbedRecordWithMedia::Main embed_AppBskyEmbedRecordWithMedia_Main;
    // union end : embed
    QString createdAt; // datetime
};
}

// app.bsky.feed.repost
namespace AppBskyFeedRepost {
struct Main
{
    ComAtprotoRepoStrongRef::Main subject;
    QString createdAt; // datetime
};
}

// app.bsky.graph.block
namespace AppBskyGraphBlock {
struct Main
{
    QString subject; // did
    QString createdAt; // datetime
};
}

// app.bsky.graph.follow
namespace AppBskyGraphFollow {
struct Main
{
    QString subject; // did
    QString createdAt; // datetime
};
}

// app.bsky.graph.list
namespace AppBskyGraphList {
struct Main
{
    AppBskyGraphDefs::ListPurpose purpose;
    QString name; //
    QString description; //
    QList<AppBskyRichtextFacet::Main> descriptionFacets;
    QString createdAt; // datetime
};
}

// app.bsky.graph.listitem
namespace AppBskyGraphListitem {
struct Main
{
    QString subject; // did
    QString list; // at-uri
    QString createdAt; // datetime
};
}

// app.bsky.notification.listNotifications
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

// com.atproto.server.defs
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

// com.atproto.moderation.defs
namespace ComAtprotoModerationDefs {
typedef QString ReasonType;
}

// com.atproto.admin.defs
namespace ComAtprotoAdminDefs {
enum class ReportViewDetailSubjectType : int {
    none,
    subject_RepoView,
    subject_RepoViewNotFound,
    subject_RecordView,
    subject_RecordViewNotFound,
};
enum class ActionViewDetailSubjectType : int {
    none,
    subject_RepoView,
    subject_RepoViewNotFound,
    subject_RecordView,
    subject_RecordViewNotFound,
};
enum class ReportViewSubjectType : int {
    none,
    subject_RepoRef,
    subject_ComAtprotoRepoStrongRef_Main,
};
enum class BlobViewDetailsType : int {
    none,
    details_ImageDetails,
    details_VideoDetails,
};
enum class ActionViewSubjectType : int {
    none,
    subject_RepoRef,
    subject_ComAtprotoRepoStrongRef_Main,
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
    ComAtprotoRepoStrongRef::Main subject_ComAtprotoRepoStrongRef_Main;
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
struct RepoViewNotFound
{
    QString did; // did
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
struct RecordViewNotFound
{
    QString uri; // at-uri
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
    ComAtprotoRepoStrongRef::Main subject_ComAtprotoRepoStrongRef_Main;
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
    RepoViewNotFound subject_RepoViewNotFound;
    RecordView subject_RecordView;
    RecordViewNotFound subject_RecordViewNotFound;
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
    RepoViewNotFound subject_RepoViewNotFound;
    RecordView subject_RecordView;
    RecordViewNotFound subject_RecordViewNotFound;
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

// com.atproto.label.subscribeLabels
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

// com.atproto.repo.applyWrites
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

// com.atproto.repo.listRecords
namespace ComAtprotoRepoListRecords {
struct Record
{
    QString uri; // at-uri
    QString cid; // cid
    QVariant value;
};
}

// com.atproto.server.createAppPassword
namespace ComAtprotoServerCreateAppPassword {
struct AppPassword
{
    QString name; //
    QString password; //
    QString createdAt; // datetime
};
}

// com.atproto.server.createInviteCodes
namespace ComAtprotoServerCreateInviteCodes {
struct AccountCodes
{
    QString account; //
};
}

// com.atproto.server.describeServer
namespace ComAtprotoServerDescribeServer {
struct Links
{
    QString privacyPolicy; //
    QString termsOfService; //
};
}

// com.atproto.server.listAppPasswords
namespace ComAtprotoServerListAppPasswords {
struct AppPassword
{
    QString name; //
    QString createdAt; // datetime
};
}

// com.atproto.sync.listRepos
namespace ComAtprotoSyncListRepos {
struct Repo
{
    QString did; // did
    QString head; // cid
};
}

// com.atproto.sync.subscribeRepos
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

}
Q_DECLARE_METATYPE(AtProtocolType::AppBskyFeedPost::Main)
Q_DECLARE_METATYPE(AtProtocolType::AppBskyFeedLike::Main)
Q_DECLARE_METATYPE(AtProtocolType::AppBskyFeedRepost::Main)

#endif // LEXICONS_H