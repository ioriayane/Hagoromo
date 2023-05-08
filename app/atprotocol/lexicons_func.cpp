// This file is generated by "defs2struct.py".
// Please do not edit.

#ifndef LEXICONS_FUNC_CPP
#define LEXICONS_FUNC_CPP

#include "lexicons_func.h"
#include "lexicons_func_unknown.h"

namespace AtProtocolType {
// app.bsky.actor.defs
namespace AppBskyActorDefs {
void copyViewerState(const QJsonObject &src, AppBskyActorDefs::ViewerState &dest)
{
    if (!src.isEmpty()) {
        dest.blocking = src.value("blocking").toString();
        dest.following = src.value("following").toString();
        dest.followedBy = src.value("followedBy").toString();
    }
}
void copyProfileViewBasic(const QJsonObject &src, AppBskyActorDefs::ProfileViewBasic &dest)
{
    if (!src.isEmpty()) {
        dest.did = src.value("did").toString();
        dest.handle = src.value("handle").toString();
        dest.displayName = src.value("displayName").toString();
        dest.avatar = src.value("avatar").toString();
        copyViewerState(src.value("viewer").toObject(), dest.viewer);
        for (const auto &s : src.value("labels").toArray()) {
            ComAtprotoLabelDefs::Label child;
            ComAtprotoLabelDefs::copyLabel(s.toObject(), child);
            dest.labels.append(child);
        }
    }
}
void copyProfileView(const QJsonObject &src, AppBskyActorDefs::ProfileView &dest)
{
    if (!src.isEmpty()) {
        dest.did = src.value("did").toString();
        dest.handle = src.value("handle").toString();
        dest.displayName = src.value("displayName").toString();
        dest.description = src.value("description").toString();
        dest.avatar = src.value("avatar").toString();
        dest.indexedAt = src.value("indexedAt").toString();
        copyViewerState(src.value("viewer").toObject(), dest.viewer);
        for (const auto &s : src.value("labels").toArray()) {
            ComAtprotoLabelDefs::Label child;
            ComAtprotoLabelDefs::copyLabel(s.toObject(), child);
            dest.labels.append(child);
        }
    }
}
void copyProfileViewDetailed(const QJsonObject &src, AppBskyActorDefs::ProfileViewDetailed &dest)
{
    if (!src.isEmpty()) {
        dest.did = src.value("did").toString();
        dest.handle = src.value("handle").toString();
        dest.displayName = src.value("displayName").toString();
        dest.description = src.value("description").toString();
        dest.avatar = src.value("avatar").toString();
        dest.banner = src.value("banner").toString();
        dest.followersCount = src.value("followersCount").toInt();
        dest.followsCount = src.value("followsCount").toInt();
        dest.postsCount = src.value("postsCount").toInt();
        dest.indexedAt = src.value("indexedAt").toString();
        copyViewerState(src.value("viewer").toObject(), dest.viewer);
        for (const auto &s : src.value("labels").toArray()) {
            ComAtprotoLabelDefs::Label child;
            ComAtprotoLabelDefs::copyLabel(s.toObject(), child);
            dest.labels.append(child);
        }
    }
}
}
// com.atproto.label.defs
namespace ComAtprotoLabelDefs {
void copyLabel(const QJsonObject &src, ComAtprotoLabelDefs::Label &dest)
{
    if (!src.isEmpty()) {
        dest.src = src.value("src").toString();
        dest.uri = src.value("uri").toString();
        dest.cid = src.value("cid").toString();
        dest.val = src.value("val").toString();
        dest.cts = src.value("cts").toString();
    }
}
}
// app.bsky.embed.external
namespace AppBskyEmbedExternal {
void copyExternal(const QJsonObject &src, AppBskyEmbedExternal::External &dest)
{
    if (!src.isEmpty()) {
        dest.uri = src.value("uri").toString();
        dest.title = src.value("title").toString();
        dest.description = src.value("description").toString();
    }
}
void copyMain(const QJsonObject &src, AppBskyEmbedExternal::Main &dest)
{
    if (!src.isEmpty()) {
        copyExternal(src.value("external").toObject(), dest.external);
    }
}
void copyViewExternal(const QJsonObject &src, AppBskyEmbedExternal::ViewExternal &dest)
{
    if (!src.isEmpty()) {
        dest.uri = src.value("uri").toString();
        dest.title = src.value("title").toString();
        dest.description = src.value("description").toString();
        dest.thumb = src.value("thumb").toString();
    }
}
void copyView(const QJsonObject &src, AppBskyEmbedExternal::View &dest)
{
    if (!src.isEmpty()) {
        copyViewExternal(src.value("external").toObject(), dest.external);
    }
}
}
// app.bsky.embed.images
namespace AppBskyEmbedImages {
void copyImage(const QJsonObject &src, AppBskyEmbedImages::Image &dest)
{
    if (!src.isEmpty()) {
        dest.alt = src.value("alt").toString();
    }
}
void copyMain(const QJsonObject &src, AppBskyEmbedImages::Main &dest)
{
    if (!src.isEmpty()) {
        for (const auto &s : src.value("images").toArray()) {
            Image child;
            copyImage(s.toObject(), child);
            dest.images.append(child);
        }
    }
}
void copyViewImage(const QJsonObject &src, AppBskyEmbedImages::ViewImage &dest)
{
    if (!src.isEmpty()) {
        dest.thumb = src.value("thumb").toString();
        dest.fullsize = src.value("fullsize").toString();
        dest.alt = src.value("alt").toString();
    }
}
void copyView(const QJsonObject &src, AppBskyEmbedImages::View &dest)
{
    if (!src.isEmpty()) {
        for (const auto &s : src.value("images").toArray()) {
            ViewImage child;
            copyViewImage(s.toObject(), child);
            dest.images.append(child);
        }
    }
}
}
// app.bsky.embed.record
namespace AppBskyEmbedRecord {
void copyMain(const QJsonObject &src, AppBskyEmbedRecord::Main &dest)
{
    if (!src.isEmpty()) {
        // ref record com.atproto.repo.strongRef
    }
}
void copyViewRecord(const QJsonObject &src, AppBskyEmbedRecord::ViewRecord &dest)
{
    if (!src.isEmpty()) {
        dest.uri = src.value("uri").toString();
        dest.cid = src.value("cid").toString();
        AppBskyActorDefs::copyProfileViewBasic(src.value("author").toObject(), dest.author);
        LexiconsTypeUnknown::copyUnknown(src.value("value").toObject(), dest.value);
        for (const auto &s : src.value("labels").toArray()) {
            ComAtprotoLabelDefs::Label child;
            ComAtprotoLabelDefs::copyLabel(s.toObject(), child);
            dest.labels.append(child);
        }
        // array<union> embeds
        dest.indexedAt = src.value("indexedAt").toString();
    }
}
void copyViewNotFound(const QJsonObject &src, AppBskyEmbedRecord::ViewNotFound &dest)
{
    if (!src.isEmpty()) {
        dest.uri = src.value("uri").toString();
    }
}
void copyViewBlocked(const QJsonObject &src, AppBskyEmbedRecord::ViewBlocked &dest)
{
    if (!src.isEmpty()) {
        dest.uri = src.value("uri").toString();
    }
}
void copyView(const QJsonObject &src, AppBskyEmbedRecord::View &dest)
{
    if (!src.isEmpty()) {
        QString record_type = src.value("record").toObject().value("$type").toString();
        if (record_type == QStringLiteral("app.bsky.embed.record#viewRecord")) {
            dest.record_type = AppBskyEmbedRecord::ViewRecordType::record_ViewRecord;
            AppBskyEmbedRecord::copyViewRecord(src.value("record").toObject(),
                                               dest.record_ViewRecord);
        }
        if (record_type == QStringLiteral("app.bsky.embed.record#viewNotFound")) {
            dest.record_type = AppBskyEmbedRecord::ViewRecordType::record_ViewNotFound;
            AppBskyEmbedRecord::copyViewNotFound(src.value("record").toObject(),
                                                 dest.record_ViewNotFound);
        }
        if (record_type == QStringLiteral("app.bsky.embed.record#viewBlocked")) {
            dest.record_type = AppBskyEmbedRecord::ViewRecordType::record_ViewBlocked;
            AppBskyEmbedRecord::copyViewBlocked(src.value("record").toObject(),
                                                dest.record_ViewBlocked);
        }
    }
}
}
// app.bsky.embed.recordWithMedia
namespace AppBskyEmbedRecordWithMedia {
void copyView(const QJsonObject &src, AppBskyEmbedRecordWithMedia::View &dest)
{
    if (!src.isEmpty()) {
        // ref *record app.bsky.embed.record#view
        QString media_type = src.value("media").toObject().value("$type").toString();
        if (media_type == QStringLiteral("app.bsky.embed.images#view")) {
            dest.media_type =
                    AppBskyEmbedRecordWithMedia::ViewMediaType::media_AppBskyEmbedImages_View;
            AppBskyEmbedImages::copyView(src.value("media").toObject(),
                                         dest.media_AppBskyEmbedImages_View);
        }
        if (media_type == QStringLiteral("app.bsky.embed.external#view")) {
            dest.media_type =
                    AppBskyEmbedRecordWithMedia::ViewMediaType::media_AppBskyEmbedExternal_View;
            AppBskyEmbedExternal::copyView(src.value("media").toObject(),
                                           dest.media_AppBskyEmbedExternal_View);
        }
    }
}
void copyMain(const QJsonObject &src, AppBskyEmbedRecordWithMedia::Main &dest)
{
    if (!src.isEmpty()) {
        // ref record app.bsky.embed.record
        QString media_type = src.value("media").toObject().value("$type").toString();
        // union media app.bsky.embed.images
        // union media app.bsky.embed.external
    }
}
}
// app.bsky.feed.defs
namespace AppBskyFeedDefs {
void copyViewerState(const QJsonObject &src, AppBskyFeedDefs::ViewerState &dest)
{
    if (!src.isEmpty()) {
        dest.repost = src.value("repost").toString();
        dest.like = src.value("like").toString();
    }
}
void copyPostView(const QJsonObject &src, AppBskyFeedDefs::PostView &dest)
{
    if (!src.isEmpty()) {
        dest.uri = src.value("uri").toString();
        dest.cid = src.value("cid").toString();
        AppBskyActorDefs::copyProfileViewBasic(src.value("author").toObject(), dest.author);
        LexiconsTypeUnknown::copyUnknown(src.value("record").toObject(), dest.record);
        QString embed_type = src.value("embed").toObject().value("$type").toString();
        if (embed_type == QStringLiteral("app.bsky.embed.images#view")) {
            dest.embed_type = AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedImages_View;
            AppBskyEmbedImages::copyView(src.value("embed").toObject(),
                                         dest.embed_AppBskyEmbedImages_View);
        }
        if (embed_type == QStringLiteral("app.bsky.embed.external#view")) {
            dest.embed_type = AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedExternal_View;
            AppBskyEmbedExternal::copyView(src.value("embed").toObject(),
                                           dest.embed_AppBskyEmbedExternal_View);
        }
        if (embed_type == QStringLiteral("app.bsky.embed.record#view")) {
            dest.embed_type = AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedRecord_View;
            AppBskyEmbedRecord::copyView(src.value("embed").toObject(),
                                         dest.embed_AppBskyEmbedRecord_View);
        }
        if (embed_type == QStringLiteral("app.bsky.embed.recordWithMedia#view")) {
            dest.embed_type =
                    AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedRecordWithMedia_View;
            AppBskyEmbedRecordWithMedia::copyView(src.value("embed").toObject(),
                                                  dest.embed_AppBskyEmbedRecordWithMedia_View);
        }
        dest.replyCount = src.value("replyCount").toInt();
        dest.repostCount = src.value("repostCount").toInt();
        dest.likeCount = src.value("likeCount").toInt();
        dest.indexedAt = src.value("indexedAt").toString();
        copyViewerState(src.value("viewer").toObject(), dest.viewer);
        for (const auto &s : src.value("labels").toArray()) {
            ComAtprotoLabelDefs::Label child;
            ComAtprotoLabelDefs::copyLabel(s.toObject(), child);
            dest.labels.append(child);
        }
    }
}
void copyReplyRef(const QJsonObject &src, AppBskyFeedDefs::ReplyRef &dest)
{
    if (!src.isEmpty()) {
        AppBskyFeedDefs::copyPostView(src.value("root").toObject(), dest.root);
        AppBskyFeedDefs::copyPostView(src.value("parent").toObject(), dest.parent);
    }
}
void copyReasonRepost(const QJsonObject &src, AppBskyFeedDefs::ReasonRepost &dest)
{
    if (!src.isEmpty()) {
        AppBskyActorDefs::copyProfileViewBasic(src.value("by").toObject(), dest.by);
        dest.indexedAt = src.value("indexedAt").toString();
    }
}
void copyFeedViewPost(const QJsonObject &src, AppBskyFeedDefs::FeedViewPost &dest)
{
    if (!src.isEmpty()) {
        AppBskyFeedDefs::copyPostView(src.value("post").toObject(), dest.post);
        copyReplyRef(src.value("reply").toObject(), dest.reply);
        QString reason_type = src.value("reason").toObject().value("$type").toString();
        if (reason_type == QStringLiteral("app.bsky.feed.defs#reasonRepost")) {
            dest.reason_type = AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost;
            AppBskyFeedDefs::copyReasonRepost(src.value("reason").toObject(),
                                              dest.reason_ReasonRepost);
        }
    }
}
void copyNotFoundPost(const QJsonObject &src, AppBskyFeedDefs::NotFoundPost &dest)
{
    if (!src.isEmpty()) {
        dest.uri = src.value("uri").toString();
    }
}
void copyBlockedPost(const QJsonObject &src, AppBskyFeedDefs::BlockedPost &dest)
{
    if (!src.isEmpty()) {
        dest.uri = src.value("uri").toString();
    }
}
void copyThreadViewPost(const QJsonObject &src, AppBskyFeedDefs::ThreadViewPost &dest)
{
    if (!src.isEmpty()) {
        copyPostView(src.value("post").toObject(), dest.post);
        QString parent_type = src.value("parent").toObject().value("$type").toString();
        // union *parent #threadViewPost
        if (parent_type == QStringLiteral("app.bsky.feed.defs#notFoundPost")) {
            dest.parent_type = AppBskyFeedDefs::ThreadViewPostParentType::parent_NotFoundPost;
            AppBskyFeedDefs::copyNotFoundPost(src.value("parent").toObject(),
                                              dest.parent_NotFoundPost);
        }
        if (parent_type == QStringLiteral("app.bsky.feed.defs#blockedPost")) {
            dest.parent_type = AppBskyFeedDefs::ThreadViewPostParentType::parent_BlockedPost;
            AppBskyFeedDefs::copyBlockedPost(src.value("parent").toObject(),
                                             dest.parent_BlockedPost);
        }
        // array<union> replies
    }
}
}
// app.bsky.feed.getLikes
namespace AppBskyFeedGetLikes {
void copyLike(const QJsonObject &src, AppBskyFeedGetLikes::Like &dest)
{
    if (!src.isEmpty()) {
        dest.indexedAt = src.value("indexedAt").toString();
        dest.createdAt = src.value("createdAt").toString();
        AppBskyActorDefs::copyProfileView(src.value("actor").toObject(), dest.actor);
    }
}
}
// app.bsky.feed.post
namespace AppBskyFeedPost {
void copyTextSlice(const QJsonObject &src, AppBskyFeedPost::TextSlice &dest)
{
    if (!src.isEmpty()) {
        dest.start = src.value("start").toInt();
        dest.end = src.value("end").toInt();
    }
}
void copyEntity(const QJsonObject &src, AppBskyFeedPost::Entity &dest)
{
    if (!src.isEmpty()) {
        copyTextSlice(src.value("index").toObject(), dest.index);
        dest.type = src.value("type").toString();
        dest.value = src.value("value").toString();
    }
}
void copyReplyRef(const QJsonObject &src, AppBskyFeedPost::ReplyRef &dest)
{
    if (!src.isEmpty()) {
        // ref root com.atproto.repo.strongRef
        // ref parent com.atproto.repo.strongRef
    }
}
}
// app.bsky.notification.listNotifications
namespace AppBskyNotificationListNotifications {
void copyNotification(const QJsonObject &src,
                      AppBskyNotificationListNotifications::Notification &dest)
{
    if (!src.isEmpty()) {
        dest.uri = src.value("uri").toString();
        dest.cid = src.value("cid").toString();
        AppBskyActorDefs::copyProfileView(src.value("author").toObject(), dest.author);
        dest.reason = src.value("reason").toString();
        dest.reasonSubject = src.value("reasonSubject").toString();
        LexiconsTypeUnknown::copyUnknown(src.value("record").toObject(), dest.record);
        dest.indexedAt = src.value("indexedAt").toString();
        for (const auto &s : src.value("labels").toArray()) {
            ComAtprotoLabelDefs::Label child;
            ComAtprotoLabelDefs::copyLabel(s.toObject(), child);
            dest.labels.append(child);
        }
    }
}
}
// app.bsky.richtext.facet
namespace AppBskyRichtextFacet {
void copyByteSlice(const QJsonObject &src, AppBskyRichtextFacet::ByteSlice &dest)
{
    if (!src.isEmpty()) {
        dest.byteStart = src.value("byteStart").toInt();
        dest.byteEnd = src.value("byteEnd").toInt();
    }
}
void copyMention(const QJsonObject &src, AppBskyRichtextFacet::Mention &dest)
{
    if (!src.isEmpty()) {
        dest.did = src.value("did").toString();
    }
}
void copyLink(const QJsonObject &src, AppBskyRichtextFacet::Link &dest)
{
    if (!src.isEmpty()) {
        dest.uri = src.value("uri").toString();
    }
}
void copyMain(const QJsonObject &src, AppBskyRichtextFacet::Main &dest)
{
    if (!src.isEmpty()) {
        copyByteSlice(src.value("index").toObject(), dest.index);
        // array<union> features
    }
}
}
// com.atproto.admin.defs
namespace ComAtprotoAdminDefs {
void copyActionType(const QJsonValue &src, ComAtprotoAdminDefs::ActionType &dest)
{
    dest = src.toString();
}
void copyRepoRef(const QJsonObject &src, ComAtprotoAdminDefs::RepoRef &dest)
{
    if (!src.isEmpty()) {
        dest.did = src.value("did").toString();
    }
}
void copyActionReversal(const QJsonObject &src, ComAtprotoAdminDefs::ActionReversal &dest)
{
    if (!src.isEmpty()) {
        dest.reason = src.value("reason").toString();
        dest.createdBy = src.value("createdBy").toString();
        dest.createdAt = src.value("createdAt").toString();
    }
}
void copyActionView(const QJsonObject &src, ComAtprotoAdminDefs::ActionView &dest)
{
    if (!src.isEmpty()) {
        dest.id = src.value("id").toInt();
        copyActionType(src.value("action"), dest.action);
        QString subject_type = src.value("subject").toObject().value("$type").toString();
        if (subject_type == QStringLiteral("com.atproto.admin.defs#repoRef")) {
            dest.subject_type = ComAtprotoAdminDefs::ActionViewSubjectType::subject_RepoRef;
            ComAtprotoAdminDefs::copyRepoRef(src.value("subject").toObject(), dest.subject_RepoRef);
        }
        // union subject com.atproto.repo.strongRef
        dest.reason = src.value("reason").toString();
        dest.createdBy = src.value("createdBy").toString();
        dest.createdAt = src.value("createdAt").toString();
        copyActionReversal(src.value("reversal").toObject(), dest.reversal);
    }
}
void copyActionViewCurrent(const QJsonObject &src, ComAtprotoAdminDefs::ActionViewCurrent &dest)
{
    if (!src.isEmpty()) {
        dest.id = src.value("id").toInt();
        copyActionType(src.value("action"), dest.action);
    }
}
void copyModeration(const QJsonObject &src, ComAtprotoAdminDefs::Moderation &dest)
{
    if (!src.isEmpty()) {
        copyActionViewCurrent(src.value("currentAction").toObject(), dest.currentAction);
    }
}
void copyRepoView(const QJsonObject &src, ComAtprotoAdminDefs::RepoView &dest)
{
    if (!src.isEmpty()) {
        dest.did = src.value("did").toString();
        dest.handle = src.value("handle").toString();
        dest.email = src.value("email").toString();
        dest.indexedAt = src.value("indexedAt").toString();
        copyModeration(src.value("moderation").toObject(), dest.moderation);
        ComAtprotoServerDefs::copyInviteCode(src.value("invitedBy").toObject(), dest.invitedBy);
    }
}
void copyRecordView(const QJsonObject &src, ComAtprotoAdminDefs::RecordView &dest)
{
    if (!src.isEmpty()) {
        dest.uri = src.value("uri").toString();
        dest.cid = src.value("cid").toString();
        LexiconsTypeUnknown::copyUnknown(src.value("value").toObject(), dest.value);
        dest.indexedAt = src.value("indexedAt").toString();
        copyModeration(src.value("moderation").toObject(), dest.moderation);
        copyRepoView(src.value("repo").toObject(), dest.repo);
    }
}
void copyImageDetails(const QJsonObject &src, ComAtprotoAdminDefs::ImageDetails &dest)
{
    if (!src.isEmpty()) {
        dest.width = src.value("width").toInt();
        dest.height = src.value("height").toInt();
    }
}
void copyVideoDetails(const QJsonObject &src, ComAtprotoAdminDefs::VideoDetails &dest)
{
    if (!src.isEmpty()) {
        dest.width = src.value("width").toInt();
        dest.height = src.value("height").toInt();
        dest.length = src.value("length").toInt();
    }
}
void copyBlobView(const QJsonObject &src, ComAtprotoAdminDefs::BlobView &dest)
{
    if (!src.isEmpty()) {
        dest.cid = src.value("cid").toString();
        dest.mimeType = src.value("mimeType").toString();
        dest.size = src.value("size").toInt();
        dest.createdAt = src.value("createdAt").toString();
        QString details_type = src.value("details").toObject().value("$type").toString();
        if (details_type == QStringLiteral("com.atproto.admin.defs#imageDetails")) {
            dest.details_type = ComAtprotoAdminDefs::BlobViewDetailsType::details_ImageDetails;
            ComAtprotoAdminDefs::copyImageDetails(src.value("details").toObject(),
                                                  dest.details_ImageDetails);
        }
        if (details_type == QStringLiteral("com.atproto.admin.defs#videoDetails")) {
            dest.details_type = ComAtprotoAdminDefs::BlobViewDetailsType::details_VideoDetails;
            ComAtprotoAdminDefs::copyVideoDetails(src.value("details").toObject(),
                                                  dest.details_VideoDetails);
        }
        copyModeration(src.value("moderation").toObject(), dest.moderation);
    }
}
void copyReportView(const QJsonObject &src, ComAtprotoAdminDefs::ReportView &dest)
{
    if (!src.isEmpty()) {
        dest.id = src.value("id").toInt();
        ComAtprotoModerationDefs::copyReasonType(src.value("reasonType"), dest.reasonType);
        dest.reason = src.value("reason").toString();
        QString subject_type = src.value("subject").toObject().value("$type").toString();
        if (subject_type == QStringLiteral("com.atproto.admin.defs#repoRef")) {
            dest.subject_type = ComAtprotoAdminDefs::ReportViewSubjectType::subject_RepoRef;
            ComAtprotoAdminDefs::copyRepoRef(src.value("subject").toObject(), dest.subject_RepoRef);
        }
        // union subject com.atproto.repo.strongRef
        dest.reportedBy = src.value("reportedBy").toString();
        dest.createdAt = src.value("createdAt").toString();
    }
}
void copyActionViewDetail(const QJsonObject &src, ComAtprotoAdminDefs::ActionViewDetail &dest)
{
    if (!src.isEmpty()) {
        dest.id = src.value("id").toInt();
        copyActionType(src.value("action"), dest.action);
        QString subject_type = src.value("subject").toObject().value("$type").toString();
        if (subject_type == QStringLiteral("com.atproto.admin.defs#repoView")) {
            dest.subject_type = ComAtprotoAdminDefs::ActionViewDetailSubjectType::subject_RepoView;
            ComAtprotoAdminDefs::copyRepoView(src.value("subject").toObject(),
                                              dest.subject_RepoView);
        }
        if (subject_type == QStringLiteral("com.atproto.admin.defs#recordView")) {
            dest.subject_type =
                    ComAtprotoAdminDefs::ActionViewDetailSubjectType::subject_RecordView;
            ComAtprotoAdminDefs::copyRecordView(src.value("subject").toObject(),
                                                dest.subject_RecordView);
        }
        for (const auto &s : src.value("subjectBlobs").toArray()) {
            BlobView child;
            copyBlobView(s.toObject(), child);
            dest.subjectBlobs.append(child);
        }
        dest.reason = src.value("reason").toString();
        dest.createdBy = src.value("createdBy").toString();
        dest.createdAt = src.value("createdAt").toString();
        copyActionReversal(src.value("reversal").toObject(), dest.reversal);
        for (const auto &s : src.value("resolvedReports").toArray()) {
            ReportView child;
            copyReportView(s.toObject(), child);
            dest.resolvedReports.append(child);
        }
    }
}
void copyReportViewDetail(const QJsonObject &src, ComAtprotoAdminDefs::ReportViewDetail &dest)
{
    if (!src.isEmpty()) {
        dest.id = src.value("id").toInt();
        ComAtprotoModerationDefs::copyReasonType(src.value("reasonType"), dest.reasonType);
        dest.reason = src.value("reason").toString();
        QString subject_type = src.value("subject").toObject().value("$type").toString();
        if (subject_type == QStringLiteral("com.atproto.admin.defs#repoView")) {
            dest.subject_type = ComAtprotoAdminDefs::ReportViewDetailSubjectType::subject_RepoView;
            ComAtprotoAdminDefs::copyRepoView(src.value("subject").toObject(),
                                              dest.subject_RepoView);
        }
        if (subject_type == QStringLiteral("com.atproto.admin.defs#recordView")) {
            dest.subject_type =
                    ComAtprotoAdminDefs::ReportViewDetailSubjectType::subject_RecordView;
            ComAtprotoAdminDefs::copyRecordView(src.value("subject").toObject(),
                                                dest.subject_RecordView);
        }
        dest.reportedBy = src.value("reportedBy").toString();
        dest.createdAt = src.value("createdAt").toString();
        for (const auto &s : src.value("resolvedByActions").toArray()) {
            ComAtprotoAdminDefs::ActionView child;
            ComAtprotoAdminDefs::copyActionView(s.toObject(), child);
            dest.resolvedByActions.append(child);
        }
    }
}
void copyModerationDetail(const QJsonObject &src, ComAtprotoAdminDefs::ModerationDetail &dest)
{
    if (!src.isEmpty()) {
        copyActionViewCurrent(src.value("currentAction").toObject(), dest.currentAction);
        for (const auto &s : src.value("actions").toArray()) {
            ActionView child;
            copyActionView(s.toObject(), child);
            dest.actions.append(child);
        }
        for (const auto &s : src.value("reports").toArray()) {
            ReportView child;
            copyReportView(s.toObject(), child);
            dest.reports.append(child);
        }
    }
}
void copyRepoViewDetail(const QJsonObject &src, ComAtprotoAdminDefs::RepoViewDetail &dest)
{
    if (!src.isEmpty()) {
        dest.did = src.value("did").toString();
        dest.handle = src.value("handle").toString();
        dest.email = src.value("email").toString();
        dest.indexedAt = src.value("indexedAt").toString();
        copyModerationDetail(src.value("moderation").toObject(), dest.moderation);
        for (const auto &s : src.value("labels").toArray()) {
            ComAtprotoLabelDefs::Label child;
            ComAtprotoLabelDefs::copyLabel(s.toObject(), child);
            dest.labels.append(child);
        }
        ComAtprotoServerDefs::copyInviteCode(src.value("invitedBy").toObject(), dest.invitedBy);
        for (const auto &s : src.value("invites").toArray()) {
            ComAtprotoServerDefs::InviteCode child;
            ComAtprotoServerDefs::copyInviteCode(s.toObject(), child);
            dest.invites.append(child);
        }
    }
}
void copyRecordViewDetail(const QJsonObject &src, ComAtprotoAdminDefs::RecordViewDetail &dest)
{
    if (!src.isEmpty()) {
        dest.uri = src.value("uri").toString();
        dest.cid = src.value("cid").toString();
        LexiconsTypeUnknown::copyUnknown(src.value("value").toObject(), dest.value);
        for (const auto &s : src.value("blobs").toArray()) {
            BlobView child;
            copyBlobView(s.toObject(), child);
            dest.blobs.append(child);
        }
        for (const auto &s : src.value("labels").toArray()) {
            ComAtprotoLabelDefs::Label child;
            ComAtprotoLabelDefs::copyLabel(s.toObject(), child);
            dest.labels.append(child);
        }
        dest.indexedAt = src.value("indexedAt").toString();
        copyModerationDetail(src.value("moderation").toObject(), dest.moderation);
        copyRepoView(src.value("repo").toObject(), dest.repo);
    }
}
}
// com.atproto.server.defs
namespace ComAtprotoServerDefs {
void copyInviteCodeUse(const QJsonObject &src, ComAtprotoServerDefs::InviteCodeUse &dest)
{
    if (!src.isEmpty()) {
        dest.usedBy = src.value("usedBy").toString();
        dest.usedAt = src.value("usedAt").toString();
    }
}
void copyInviteCode(const QJsonObject &src, ComAtprotoServerDefs::InviteCode &dest)
{
    if (!src.isEmpty()) {
        dest.code = src.value("code").toString();
        dest.available = src.value("available").toInt();
        dest.forAccount = src.value("forAccount").toString();
        dest.createdBy = src.value("createdBy").toString();
        dest.createdAt = src.value("createdAt").toString();
        for (const auto &s : src.value("uses").toArray()) {
            InviteCodeUse child;
            copyInviteCodeUse(s.toObject(), child);
            dest.uses.append(child);
        }
    }
}
}
// com.atproto.moderation.defs
namespace ComAtprotoModerationDefs {
void copyReasonType(const QJsonValue &src, ComAtprotoModerationDefs::ReasonType &dest)
{
    dest = src.toString();
}
}
// com.atproto.label.subscribeLabels
namespace ComAtprotoLabelSubscribeLabels {
void copyLabels(const QJsonObject &src, ComAtprotoLabelSubscribeLabels::Labels &dest)
{
    if (!src.isEmpty()) {
        dest.seq = src.value("seq").toInt();
        for (const auto &s : src.value("labels").toArray()) {
            ComAtprotoLabelDefs::Label child;
            ComAtprotoLabelDefs::copyLabel(s.toObject(), child);
            dest.labels.append(child);
        }
    }
}
void copyInfo(const QJsonObject &src, ComAtprotoLabelSubscribeLabels::Info &dest)
{
    if (!src.isEmpty()) {
        dest.name = src.value("name").toString();
        dest.message = src.value("message").toString();
    }
}
}
// com.atproto.repo.applyWrites
namespace ComAtprotoRepoApplyWrites {
void copyCreate(const QJsonObject &src, ComAtprotoRepoApplyWrites::Create &dest)
{
    if (!src.isEmpty()) {
        dest.collection = src.value("collection").toString();
        dest.rkey = src.value("rkey").toString();
        LexiconsTypeUnknown::copyUnknown(src.value("value").toObject(), dest.value);
    }
}
void copyUpdate(const QJsonObject &src, ComAtprotoRepoApplyWrites::Update &dest)
{
    if (!src.isEmpty()) {
        dest.collection = src.value("collection").toString();
        dest.rkey = src.value("rkey").toString();
        LexiconsTypeUnknown::copyUnknown(src.value("value").toObject(), dest.value);
    }
}
void copyDelete(const QJsonObject &src, ComAtprotoRepoApplyWrites::Delete &dest)
{
    if (!src.isEmpty()) {
        dest.collection = src.value("collection").toString();
        dest.rkey = src.value("rkey").toString();
    }
}
}
// com.atproto.repo.listRecords
namespace ComAtprotoRepoListRecords {
void copyRecord(const QJsonObject &src, ComAtprotoRepoListRecords::Record &dest)
{
    if (!src.isEmpty()) {
        dest.uri = src.value("uri").toString();
        dest.cid = src.value("cid").toString();
        LexiconsTypeUnknown::copyUnknown(src.value("value").toObject(), dest.value);
    }
}
}
// com.atproto.repo.strongRef
namespace ComAtprotoRepoStrongRef {
void copyMain(const QJsonObject &src, ComAtprotoRepoStrongRef::Main &dest)
{
    if (!src.isEmpty()) {
        dest.uri = src.value("uri").toString();
        dest.cid = src.value("cid").toString();
    }
}
}
// com.atproto.server.createAppPassword
namespace ComAtprotoServerCreateAppPassword {
void copyAppPassword(const QJsonObject &src, ComAtprotoServerCreateAppPassword::AppPassword &dest)
{
    if (!src.isEmpty()) {
        dest.name = src.value("name").toString();
        dest.password = src.value("password").toString();
        dest.createdAt = src.value("createdAt").toString();
    }
}
}
// com.atproto.server.createInviteCodes
namespace ComAtprotoServerCreateInviteCodes {
void copyAccountCodes(const QJsonObject &src, ComAtprotoServerCreateInviteCodes::AccountCodes &dest)
{
    if (!src.isEmpty()) {
        dest.account = src.value("account").toString();
    }
}
}
// com.atproto.server.describeServer
namespace ComAtprotoServerDescribeServer {
void copyLinks(const QJsonObject &src, ComAtprotoServerDescribeServer::Links &dest)
{
    if (!src.isEmpty()) {
        dest.privacyPolicy = src.value("privacyPolicy").toString();
        dest.termsOfService = src.value("termsOfService").toString();
    }
}
}
// com.atproto.server.listAppPasswords
namespace ComAtprotoServerListAppPasswords {
void copyAppPassword(const QJsonObject &src, ComAtprotoServerListAppPasswords::AppPassword &dest)
{
    if (!src.isEmpty()) {
        dest.name = src.value("name").toString();
        dest.createdAt = src.value("createdAt").toString();
    }
}
}
// com.atproto.sync.listRepos
namespace ComAtprotoSyncListRepos {
void copyRepo(const QJsonObject &src, ComAtprotoSyncListRepos::Repo &dest)
{
    if (!src.isEmpty()) {
        dest.did = src.value("did").toString();
        dest.head = src.value("head").toString();
    }
}
}
// com.atproto.sync.subscribeRepos
namespace ComAtprotoSyncSubscribeRepos {
void copyRepoOp(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::RepoOp &dest)
{
    if (!src.isEmpty()) {
        dest.action = src.value("action").toString();
        dest.path = src.value("path").toString();
    }
}
void copyCommit(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::Commit &dest)
{
    if (!src.isEmpty()) {
        dest.seq = src.value("seq").toInt();
        dest.repo = src.value("repo").toString();
        for (const auto &s : src.value("ops").toArray()) {
            RepoOp child;
            copyRepoOp(s.toObject(), child);
            dest.ops.append(child);
        }
        dest.time = src.value("time").toString();
    }
}
void copyHandle(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::Handle &dest)
{
    if (!src.isEmpty()) {
        dest.seq = src.value("seq").toInt();
        dest.did = src.value("did").toString();
        dest.handle = src.value("handle").toString();
        dest.time = src.value("time").toString();
    }
}
void copyMigrate(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::Migrate &dest)
{
    if (!src.isEmpty()) {
        dest.seq = src.value("seq").toInt();
        dest.did = src.value("did").toString();
        dest.migrateTo = src.value("migrateTo").toString();
        dest.time = src.value("time").toString();
    }
}
void copyTombstone(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::Tombstone &dest)
{
    if (!src.isEmpty()) {
        dest.seq = src.value("seq").toInt();
        dest.did = src.value("did").toString();
        dest.time = src.value("time").toString();
    }
}
void copyInfo(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::Info &dest)
{
    if (!src.isEmpty()) {
        dest.name = src.value("name").toString();
        dest.message = src.value("message").toString();
    }
}
}

}

#endif // LEXICONS_FUNC_CPP
