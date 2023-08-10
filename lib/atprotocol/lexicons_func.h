// This file is generated by "defs2struct.py".
// Please do not edit.

#ifndef LEXICONS_FUNC_H
#define LEXICONS_FUNC_H

#include "lexicons.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QString>
#include <QVariant>

namespace AtProtocolType {
// app.bsky.graph.defs
namespace AppBskyGraphDefs {
void copyListPurpose(const QJsonValue &src, AppBskyGraphDefs::ListPurpose &dest);
void copyListViewerState(const QJsonObject &src, AppBskyGraphDefs::ListViewerState &dest);
void copyListViewBasic(const QJsonObject &src, AppBskyGraphDefs::ListViewBasic &dest);
void copyListView(const QJsonObject &src, AppBskyGraphDefs::ListView &dest);
void copyListItemView(const QJsonObject &src, AppBskyGraphDefs::ListItemView &dest);
}
// app.bsky.actor.defs
namespace AppBskyActorDefs {
void copyViewerState(const QJsonObject &src, AppBskyActorDefs::ViewerState &dest);
void copyProfileViewBasic(const QJsonObject &src, AppBskyActorDefs::ProfileViewBasic &dest);
void copyProfileView(const QJsonObject &src, AppBskyActorDefs::ProfileView &dest);
void copyProfileViewDetailed(const QJsonObject &src, AppBskyActorDefs::ProfileViewDetailed &dest);
void copyAdultContentPref(const QJsonObject &src, AppBskyActorDefs::AdultContentPref &dest);
void copyContentLabelPref(const QJsonObject &src, AppBskyActorDefs::ContentLabelPref &dest);
void copySavedFeedsPref(const QJsonObject &src, AppBskyActorDefs::SavedFeedsPref &dest);
}
// com.atproto.label.defs
namespace ComAtprotoLabelDefs {
void copyLabel(const QJsonObject &src, ComAtprotoLabelDefs::Label &dest);
void copySelfLabel(const QJsonObject &src, ComAtprotoLabelDefs::SelfLabel &dest);
void copySelfLabels(const QJsonObject &src, ComAtprotoLabelDefs::SelfLabels &dest);
}
// app.bsky.actor.profile
namespace AppBskyActorProfile {
void copyMain(const QJsonObject &src, AppBskyActorProfile::Main &dest);
}
// app.bsky.embed.external
namespace AppBskyEmbedExternal {
void copyExternal(const QJsonObject &src, AppBskyEmbedExternal::External &dest);
void copyMain(const QJsonObject &src, AppBskyEmbedExternal::Main &dest);
void copyViewExternal(const QJsonObject &src, AppBskyEmbedExternal::ViewExternal &dest);
void copyView(const QJsonObject &src, AppBskyEmbedExternal::View &dest);
}
// app.bsky.embed.images
namespace AppBskyEmbedImages {
void copyImage(const QJsonObject &src, AppBskyEmbedImages::Image &dest);
void copyMain(const QJsonObject &src, AppBskyEmbedImages::Main &dest);
void copyViewImage(const QJsonObject &src, AppBskyEmbedImages::ViewImage &dest);
void copyView(const QJsonObject &src, AppBskyEmbedImages::View &dest);
}
// com.atproto.repo.strongRef
namespace ComAtprotoRepoStrongRef {
void copyMain(const QJsonObject &src, ComAtprotoRepoStrongRef::Main &dest);
}
// app.bsky.embed.record
namespace AppBskyEmbedRecord {
void copyMain(const QJsonObject &src, AppBskyEmbedRecord::Main &dest);
void copyViewRecord(const QJsonObject &src, AppBskyEmbedRecord::ViewRecord &dest);
void copyViewNotFound(const QJsonObject &src, AppBskyEmbedRecord::ViewNotFound &dest);
void copyViewBlocked(const QJsonObject &src, AppBskyEmbedRecord::ViewBlocked &dest);
void copyView(const QJsonObject &src, AppBskyEmbedRecord::View &dest);
}
// app.bsky.embed.recordWithMedia
namespace AppBskyEmbedRecordWithMedia {
void copyView(const QJsonObject &src, AppBskyEmbedRecordWithMedia::View &dest);
void copyMain(const QJsonObject &src, AppBskyEmbedRecordWithMedia::Main &dest);
}
// app.bsky.richtext.facet
namespace AppBskyRichtextFacet {
void copyByteSlice(const QJsonObject &src, AppBskyRichtextFacet::ByteSlice &dest);
void copyMention(const QJsonObject &src, AppBskyRichtextFacet::Mention &dest);
void copyLink(const QJsonObject &src, AppBskyRichtextFacet::Link &dest);
void copyMain(const QJsonObject &src, AppBskyRichtextFacet::Main &dest);
}
// app.bsky.feed.defs
namespace AppBskyFeedDefs {
void copyGeneratorViewerState(const QJsonObject &src, AppBskyFeedDefs::GeneratorViewerState &dest);
void copyGeneratorView(const QJsonObject &src, AppBskyFeedDefs::GeneratorView &dest);
void copyViewerState(const QJsonObject &src, AppBskyFeedDefs::ViewerState &dest);
void copyPostView(const QJsonObject &src, AppBskyFeedDefs::PostView &dest);
void copyNotFoundPost(const QJsonObject &src, AppBskyFeedDefs::NotFoundPost &dest);
void copyBlockedPost(const QJsonObject &src, AppBskyFeedDefs::BlockedPost &dest);
void copyReplyRef(const QJsonObject &src, AppBskyFeedDefs::ReplyRef &dest);
void copyReasonRepost(const QJsonObject &src, AppBskyFeedDefs::ReasonRepost &dest);
void copyFeedViewPost(const QJsonObject &src, AppBskyFeedDefs::FeedViewPost &dest);
void copyThreadViewPost(const QJsonObject &src, AppBskyFeedDefs::ThreadViewPost &dest);
void copySkeletonReasonRepost(const QJsonObject &src, AppBskyFeedDefs::SkeletonReasonRepost &dest);
void copySkeletonFeedPost(const QJsonObject &src, AppBskyFeedDefs::SkeletonFeedPost &dest);
}
// app.bsky.feed.describeFeedGenerator
namespace AppBskyFeedDescribeFeedGenerator {
void copyFeed(const QJsonObject &src, AppBskyFeedDescribeFeedGenerator::Feed &dest);
void copyLinks(const QJsonObject &src, AppBskyFeedDescribeFeedGenerator::Links &dest);
}
// app.bsky.feed.generator
namespace AppBskyFeedGenerator {
void copyMain(const QJsonObject &src, AppBskyFeedGenerator::Main &dest);
}
// app.bsky.feed.getLikes
namespace AppBskyFeedGetLikes {
void copyLike(const QJsonObject &src, AppBskyFeedGetLikes::Like &dest);
}
// app.bsky.feed.like
namespace AppBskyFeedLike {
void copyMain(const QJsonObject &src, AppBskyFeedLike::Main &dest);
}
// app.bsky.feed.post
namespace AppBskyFeedPost {
void copyTextSlice(const QJsonObject &src, AppBskyFeedPost::TextSlice &dest);
void copyEntity(const QJsonObject &src, AppBskyFeedPost::Entity &dest);
void copyReplyRef(const QJsonObject &src, AppBskyFeedPost::ReplyRef &dest);
void copyMain(const QJsonObject &src, AppBskyFeedPost::Main &dest);
}
// app.bsky.feed.repost
namespace AppBskyFeedRepost {
void copyMain(const QJsonObject &src, AppBskyFeedRepost::Main &dest);
}
// app.bsky.graph.block
namespace AppBskyGraphBlock {
void copyMain(const QJsonObject &src, AppBskyGraphBlock::Main &dest);
}
// app.bsky.graph.follow
namespace AppBskyGraphFollow {
void copyMain(const QJsonObject &src, AppBskyGraphFollow::Main &dest);
}
// app.bsky.graph.list
namespace AppBskyGraphList {
void copyMain(const QJsonObject &src, AppBskyGraphList::Main &dest);
}
// app.bsky.graph.listitem
namespace AppBskyGraphListitem {
void copyMain(const QJsonObject &src, AppBskyGraphListitem::Main &dest);
}
// app.bsky.notification.listNotifications
namespace AppBskyNotificationListNotifications {
void copyNotification(const QJsonObject &src,
                      AppBskyNotificationListNotifications::Notification &dest);
}
// com.atproto.admin.defs
namespace ComAtprotoAdminDefs {
void copyActionType(const QJsonValue &src, ComAtprotoAdminDefs::ActionType &dest);
void copyRepoRef(const QJsonObject &src, ComAtprotoAdminDefs::RepoRef &dest);
void copyActionReversal(const QJsonObject &src, ComAtprotoAdminDefs::ActionReversal &dest);
void copyActionView(const QJsonObject &src, ComAtprotoAdminDefs::ActionView &dest);
void copyActionViewCurrent(const QJsonObject &src, ComAtprotoAdminDefs::ActionViewCurrent &dest);
void copyModeration(const QJsonObject &src, ComAtprotoAdminDefs::Moderation &dest);
void copyRepoView(const QJsonObject &src, ComAtprotoAdminDefs::RepoView &dest);
void copyRepoViewNotFound(const QJsonObject &src, ComAtprotoAdminDefs::RepoViewNotFound &dest);
void copyRecordView(const QJsonObject &src, ComAtprotoAdminDefs::RecordView &dest);
void copyRecordViewNotFound(const QJsonObject &src, ComAtprotoAdminDefs::RecordViewNotFound &dest);
void copyImageDetails(const QJsonObject &src, ComAtprotoAdminDefs::ImageDetails &dest);
void copyVideoDetails(const QJsonObject &src, ComAtprotoAdminDefs::VideoDetails &dest);
void copyBlobView(const QJsonObject &src, ComAtprotoAdminDefs::BlobView &dest);
void copyReportView(const QJsonObject &src, ComAtprotoAdminDefs::ReportView &dest);
void copyActionViewDetail(const QJsonObject &src, ComAtprotoAdminDefs::ActionViewDetail &dest);
void copyReportViewDetail(const QJsonObject &src, ComAtprotoAdminDefs::ReportViewDetail &dest);
void copyModerationDetail(const QJsonObject &src, ComAtprotoAdminDefs::ModerationDetail &dest);
void copyRepoViewDetail(const QJsonObject &src, ComAtprotoAdminDefs::RepoViewDetail &dest);
void copyRecordViewDetail(const QJsonObject &src, ComAtprotoAdminDefs::RecordViewDetail &dest);
}
// com.atproto.server.defs
namespace ComAtprotoServerDefs {
void copyInviteCodeUse(const QJsonObject &src, ComAtprotoServerDefs::InviteCodeUse &dest);
void copyInviteCode(const QJsonObject &src, ComAtprotoServerDefs::InviteCode &dest);
}
// com.atproto.moderation.defs
namespace ComAtprotoModerationDefs {
void copyReasonType(const QJsonValue &src, ComAtprotoModerationDefs::ReasonType &dest);
}
// com.atproto.label.subscribeLabels
namespace ComAtprotoLabelSubscribeLabels {
void copyLabels(const QJsonObject &src, ComAtprotoLabelSubscribeLabels::Labels &dest);
void copyInfo(const QJsonObject &src, ComAtprotoLabelSubscribeLabels::Info &dest);
}
// com.atproto.repo.applyWrites
namespace ComAtprotoRepoApplyWrites {
void copyCreate(const QJsonObject &src, ComAtprotoRepoApplyWrites::Create &dest);
void copyUpdate(const QJsonObject &src, ComAtprotoRepoApplyWrites::Update &dest);
void copyDelete(const QJsonObject &src, ComAtprotoRepoApplyWrites::Delete &dest);
}
// com.atproto.repo.listRecords
namespace ComAtprotoRepoListRecords {
void copyRecord(const QJsonObject &src, ComAtprotoRepoListRecords::Record &dest);
}
// com.atproto.server.createAppPassword
namespace ComAtprotoServerCreateAppPassword {
void copyAppPassword(const QJsonObject &src, ComAtprotoServerCreateAppPassword::AppPassword &dest);
}
// com.atproto.server.createInviteCodes
namespace ComAtprotoServerCreateInviteCodes {
void copyAccountCodes(const QJsonObject &src,
                      ComAtprotoServerCreateInviteCodes::AccountCodes &dest);
}
// com.atproto.server.describeServer
namespace ComAtprotoServerDescribeServer {
void copyLinks(const QJsonObject &src, ComAtprotoServerDescribeServer::Links &dest);
}
// com.atproto.server.listAppPasswords
namespace ComAtprotoServerListAppPasswords {
void copyAppPassword(const QJsonObject &src, ComAtprotoServerListAppPasswords::AppPassword &dest);
}
// com.atproto.sync.listRepos
namespace ComAtprotoSyncListRepos {
void copyRepo(const QJsonObject &src, ComAtprotoSyncListRepos::Repo &dest);
}
// com.atproto.sync.subscribeRepos
namespace ComAtprotoSyncSubscribeRepos {
void copyRepoOp(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::RepoOp &dest);
void copyCommit(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::Commit &dest);
void copyHandle(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::Handle &dest);
void copyMigrate(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::Migrate &dest);
void copyTombstone(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::Tombstone &dest);
void copyInfo(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::Info &dest);
}

}

#endif // LEXICONS_FUNC_H
