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
// app.bsky.actor.defs
namespace AppBskyActorDefs {
void copyProfileAssociatedChat(const QJsonObject &src,
                               AppBskyActorDefs::ProfileAssociatedChat &dest);
void copyProfileAssociated(const QJsonObject &src, AppBskyActorDefs::ProfileAssociated &dest);
void copyKnownFollowers(const QJsonObject &src, AppBskyActorDefs::KnownFollowers &dest);
void copyViewerState(const QJsonObject &src, AppBskyActorDefs::ViewerState &dest);
void copyProfileViewBasic(const QJsonObject &src, AppBskyActorDefs::ProfileViewBasic &dest);
void copyProfileView(const QJsonObject &src, AppBskyActorDefs::ProfileView &dest);
void copyProfileViewDetailed(const QJsonObject &src, AppBskyActorDefs::ProfileViewDetailed &dest);
void copyAdultContentPref(const QJsonObject &src, AppBskyActorDefs::AdultContentPref &dest);
void copyContentLabelPref(const QJsonObject &src, AppBskyActorDefs::ContentLabelPref &dest);
void copySavedFeedsPref(const QJsonObject &src, AppBskyActorDefs::SavedFeedsPref &dest);
void copySavedFeed(const QJsonObject &src, AppBskyActorDefs::SavedFeed &dest);
void copySavedFeedsPrefV2(const QJsonObject &src, AppBskyActorDefs::SavedFeedsPrefV2 &dest);
void copyPersonalDetailsPref(const QJsonObject &src, AppBskyActorDefs::PersonalDetailsPref &dest);
void copyFeedViewPref(const QJsonObject &src, AppBskyActorDefs::FeedViewPref &dest);
void copyThreadViewPref(const QJsonObject &src, AppBskyActorDefs::ThreadViewPref &dest);
void copyInterestsPref(const QJsonObject &src, AppBskyActorDefs::InterestsPref &dest);
void copyMutedWordTarget(const QJsonValue &src, AppBskyActorDefs::MutedWordTarget &dest);
void copyMutedWord(const QJsonObject &src, AppBskyActorDefs::MutedWord &dest);
void copyMutedWordsPref(const QJsonObject &src, AppBskyActorDefs::MutedWordsPref &dest);
void copyHiddenPostsPref(const QJsonObject &src, AppBskyActorDefs::HiddenPostsPref &dest);
void copyBskyAppProgressGuide(const QJsonObject &src, AppBskyActorDefs::BskyAppProgressGuide &dest);
void copyBskyAppStatePref(const QJsonObject &src, AppBskyActorDefs::BskyAppStatePref &dest);
void copyLabelerPrefItem(const QJsonObject &src, AppBskyActorDefs::LabelerPrefItem &dest);
void copyLabelersPref(const QJsonObject &src, AppBskyActorDefs::LabelersPref &dest);
void copyPreferences(const QJsonArray &src, AppBskyActorDefs::Preferences &dest);
}
// app.bsky.graph.defs
namespace AppBskyGraphDefs {
void copyListPurpose(const QJsonValue &src, AppBskyGraphDefs::ListPurpose &dest);
void copyListViewerState(const QJsonObject &src, AppBskyGraphDefs::ListViewerState &dest);
void copyListViewBasic(const QJsonObject &src, AppBskyGraphDefs::ListViewBasic &dest);
void copyStarterPackViewBasic(const QJsonObject &src, AppBskyGraphDefs::StarterPackViewBasic &dest);
void copyListView(const QJsonObject &src, AppBskyGraphDefs::ListView &dest);
void copyListItemView(const QJsonObject &src, AppBskyGraphDefs::ListItemView &dest);
void copyStarterPackView(const QJsonObject &src, AppBskyGraphDefs::StarterPackView &dest);
void copyNotFoundActor(const QJsonObject &src, AppBskyGraphDefs::NotFoundActor &dest);
void copyRelationship(const QJsonObject &src, AppBskyGraphDefs::Relationship &dest);
}
// com.atproto.label.defs
namespace ComAtprotoLabelDefs {
void copyLabel(const QJsonObject &src, ComAtprotoLabelDefs::Label &dest);
void copySelfLabel(const QJsonObject &src, ComAtprotoLabelDefs::SelfLabel &dest);
void copySelfLabels(const QJsonObject &src, ComAtprotoLabelDefs::SelfLabels &dest);
void copyLabelValue(const QJsonValue &src, ComAtprotoLabelDefs::LabelValue &dest);
void copyLabelValueDefinitionStrings(const QJsonObject &src,
                                     ComAtprotoLabelDefs::LabelValueDefinitionStrings &dest);
void copyLabelValueDefinition(const QJsonObject &src,
                              ComAtprotoLabelDefs::LabelValueDefinition &dest);
}
// com.atproto.repo.strongRef
namespace ComAtprotoRepoStrongRef {
void copyMain(const QJsonObject &src, ComAtprotoRepoStrongRef::Main &dest);
}
// app.bsky.actor.profile
namespace AppBskyActorProfile {
void copyMain(const QJsonObject &src, AppBskyActorProfile::Main &dest);
}
// app.bsky.embed.defs
namespace AppBskyEmbedDefs {
void copyAspectRatio(const QJsonObject &src, AppBskyEmbedDefs::AspectRatio &dest);
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
// app.bsky.embed.record
namespace AppBskyEmbedRecord {
void copyMain(const QJsonObject &src, AppBskyEmbedRecord::Main &dest);
void copyViewRecord(const QJsonObject &src, AppBskyEmbedRecord::ViewRecord &dest);
void copyViewNotFound(const QJsonObject &src, AppBskyEmbedRecord::ViewNotFound &dest);
void copyViewBlocked(const QJsonObject &src, AppBskyEmbedRecord::ViewBlocked &dest);
void copyViewDetached(const QJsonObject &src, AppBskyEmbedRecord::ViewDetached &dest);
void copyView(const QJsonObject &src, AppBskyEmbedRecord::View &dest);
}
// app.bsky.embed.video
namespace AppBskyEmbedVideo {
void copyView(const QJsonObject &src, AppBskyEmbedVideo::View &dest);
void copyCaption(const QJsonObject &src, AppBskyEmbedVideo::Caption &dest);
void copyMain(const QJsonObject &src, AppBskyEmbedVideo::Main &dest);
}
// app.bsky.embed.recordWithMedia
namespace AppBskyEmbedRecordWithMedia {
void copyView(const QJsonObject &src, AppBskyEmbedRecordWithMedia::View &dest);
void copyMain(const QJsonObject &src, AppBskyEmbedRecordWithMedia::Main &dest);
}
// app.bsky.feed.defs
namespace AppBskyFeedDefs {
void copyBlockedAuthor(const QJsonObject &src, AppBskyFeedDefs::BlockedAuthor &dest);
void copyGeneratorViewerState(const QJsonObject &src, AppBskyFeedDefs::GeneratorViewerState &dest);
void copyGeneratorView(const QJsonObject &src, AppBskyFeedDefs::GeneratorView &dest);
void copyViewerState(const QJsonObject &src, AppBskyFeedDefs::ViewerState &dest);
void copyThreadgateView(const QJsonObject &src, AppBskyFeedDefs::ThreadgateView &dest);
void copyPostView(const QJsonObject &src, AppBskyFeedDefs::PostView &dest);
void copyNotFoundPost(const QJsonObject &src, AppBskyFeedDefs::NotFoundPost &dest);
void copyBlockedPost(const QJsonObject &src, AppBskyFeedDefs::BlockedPost &dest);
void copyReplyRef(const QJsonObject &src, AppBskyFeedDefs::ReplyRef &dest);
void copyReasonRepost(const QJsonObject &src, AppBskyFeedDefs::ReasonRepost &dest);
void copyFeedViewPost(const QJsonObject &src, AppBskyFeedDefs::FeedViewPost &dest);
void copyThreadViewPost(const QJsonObject &src, AppBskyFeedDefs::ThreadViewPost &dest);
void copySkeletonReasonRepost(const QJsonObject &src, AppBskyFeedDefs::SkeletonReasonRepost &dest);
void copySkeletonFeedPost(const QJsonObject &src, AppBskyFeedDefs::SkeletonFeedPost &dest);
void copyInteraction(const QJsonObject &src, AppBskyFeedDefs::Interaction &dest);
}
// app.bsky.richtext.facet
namespace AppBskyRichtextFacet {
void copyByteSlice(const QJsonObject &src, AppBskyRichtextFacet::ByteSlice &dest);
void copyMention(const QJsonObject &src, AppBskyRichtextFacet::Mention &dest);
void copyLink(const QJsonObject &src, AppBskyRichtextFacet::Link &dest);
void copyTag(const QJsonObject &src, AppBskyRichtextFacet::Tag &dest);
void copyMain(const QJsonObject &src, AppBskyRichtextFacet::Main &dest);
}
// app.bsky.labeler.defs
namespace AppBskyLabelerDefs {
void copyLabelerViewerState(const QJsonObject &src, AppBskyLabelerDefs::LabelerViewerState &dest);
void copyLabelerView(const QJsonObject &src, AppBskyLabelerDefs::LabelerView &dest);
void copyLabelerPolicies(const QJsonObject &src, AppBskyLabelerDefs::LabelerPolicies &dest);
void copyLabelerViewDetailed(const QJsonObject &src, AppBskyLabelerDefs::LabelerViewDetailed &dest);
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
// app.bsky.feed.postgate
namespace AppBskyFeedPostgate {
void copyDisableRule(const QJsonObject &src, AppBskyFeedPostgate::DisableRule &dest);
void copyMain(const QJsonObject &src, AppBskyFeedPostgate::Main &dest);
}
// app.bsky.feed.repost
namespace AppBskyFeedRepost {
void copyMain(const QJsonObject &src, AppBskyFeedRepost::Main &dest);
}
// app.bsky.feed.threadgate
namespace AppBskyFeedThreadgate {
void copyMentionRule(const QJsonObject &src, AppBskyFeedThreadgate::MentionRule &dest);
void copyFollowingRule(const QJsonObject &src, AppBskyFeedThreadgate::FollowingRule &dest);
void copyListRule(const QJsonObject &src, AppBskyFeedThreadgate::ListRule &dest);
void copyMain(const QJsonObject &src, AppBskyFeedThreadgate::Main &dest);
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
// app.bsky.graph.listblock
namespace AppBskyGraphListblock {
void copyMain(const QJsonObject &src, AppBskyGraphListblock::Main &dest);
}
// app.bsky.graph.listitem
namespace AppBskyGraphListitem {
void copyMain(const QJsonObject &src, AppBskyGraphListitem::Main &dest);
}
// app.bsky.graph.starterpack
namespace AppBskyGraphStarterpack {
void copyFeedItem(const QJsonObject &src, AppBskyGraphStarterpack::FeedItem &dest);
void copyMain(const QJsonObject &src, AppBskyGraphStarterpack::Main &dest);
}
// app.bsky.labeler.service
namespace AppBskyLabelerService {
void copyMain(const QJsonObject &src, AppBskyLabelerService::Main &dest);
}
// app.bsky.notification.listNotifications
namespace AppBskyNotificationListNotifications {
void copyNotification(const QJsonObject &src,
                      AppBskyNotificationListNotifications::Notification &dest);
}
// app.bsky.unspecced.defs
namespace AppBskyUnspeccedDefs {
void copySkeletonSearchPost(const QJsonObject &src, AppBskyUnspeccedDefs::SkeletonSearchPost &dest);
void copySkeletonSearchActor(const QJsonObject &src,
                             AppBskyUnspeccedDefs::SkeletonSearchActor &dest);
}
// app.bsky.unspecced.getTaggedSuggestions
namespace AppBskyUnspeccedGetTaggedSuggestions {
void copySuggestion(const QJsonObject &src, AppBskyUnspeccedGetTaggedSuggestions::Suggestion &dest);
}
// app.bsky.video.defs
namespace AppBskyVideoDefs {
void copyJobStatus(const QJsonObject &src, AppBskyVideoDefs::JobStatus &dest);
}
// chat.bsky.actor.declaration
namespace ChatBskyActorDeclaration {
void copyMain(const QJsonObject &src, ChatBskyActorDeclaration::Main &dest);
}
// chat.bsky.actor.defs
namespace ChatBskyActorDefs {
void copyProfileViewBasic(const QJsonObject &src, ChatBskyActorDefs::ProfileViewBasic &dest);
}
// chat.bsky.convo.defs
namespace ChatBskyConvoDefs {
void copyMessageRef(const QJsonObject &src, ChatBskyConvoDefs::MessageRef &dest);
void copyMessageInput(const QJsonObject &src, ChatBskyConvoDefs::MessageInput &dest);
void copyMessageViewSender(const QJsonObject &src, ChatBskyConvoDefs::MessageViewSender &dest);
void copyMessageView(const QJsonObject &src, ChatBskyConvoDefs::MessageView &dest);
void copyDeletedMessageView(const QJsonObject &src, ChatBskyConvoDefs::DeletedMessageView &dest);
void copyConvoView(const QJsonObject &src, ChatBskyConvoDefs::ConvoView &dest);
void copyLogBeginConvo(const QJsonObject &src, ChatBskyConvoDefs::LogBeginConvo &dest);
void copyLogLeaveConvo(const QJsonObject &src, ChatBskyConvoDefs::LogLeaveConvo &dest);
void copyLogCreateMessage(const QJsonObject &src, ChatBskyConvoDefs::LogCreateMessage &dest);
void copyLogDeleteMessage(const QJsonObject &src, ChatBskyConvoDefs::LogDeleteMessage &dest);
}
// chat.bsky.convo.sendMessageBatch
namespace ChatBskyConvoSendMessageBatch {
void copyBatchItem(const QJsonObject &src, ChatBskyConvoSendMessageBatch::BatchItem &dest);
}
// chat.bsky.moderation.getActorMetadata
namespace ChatBskyModerationGetActorMetadata {
void copyMetadata(const QJsonObject &src, ChatBskyModerationGetActorMetadata::Metadata &dest);
}
// com.atproto.admin.defs
namespace ComAtprotoAdminDefs {
void copyStatusAttr(const QJsonObject &src, ComAtprotoAdminDefs::StatusAttr &dest);
void copyAccountView(const QJsonObject &src, ComAtprotoAdminDefs::AccountView &dest);
void copyRepoRef(const QJsonObject &src, ComAtprotoAdminDefs::RepoRef &dest);
void copyRepoBlobRef(const QJsonObject &src, ComAtprotoAdminDefs::RepoBlobRef &dest);
}
// com.atproto.server.defs
namespace ComAtprotoServerDefs {
void copyInviteCodeUse(const QJsonObject &src, ComAtprotoServerDefs::InviteCodeUse &dest);
void copyInviteCode(const QJsonObject &src, ComAtprotoServerDefs::InviteCode &dest);
}
// com.atproto.label.subscribeLabels
namespace ComAtprotoLabelSubscribeLabels {
void copyLabels(const QJsonObject &src, ComAtprotoLabelSubscribeLabels::Labels &dest);
void copyInfo(const QJsonObject &src, ComAtprotoLabelSubscribeLabels::Info &dest);
}
// com.atproto.moderation.defs
namespace ComAtprotoModerationDefs {
void copyReasonType(const QJsonValue &src, ComAtprotoModerationDefs::ReasonType &dest);
}
// com.atproto.repo.applyWrites
namespace ComAtprotoRepoApplyWrites {
void copyCreate(const QJsonObject &src, ComAtprotoRepoApplyWrites::Create &dest);
void copyUpdate(const QJsonObject &src, ComAtprotoRepoApplyWrites::Update &dest);
void copyDelete(const QJsonObject &src, ComAtprotoRepoApplyWrites::Delete &dest);
}
// com.atproto.repo.listMissingBlobs
namespace ComAtprotoRepoListMissingBlobs {
void copyRecordBlob(const QJsonObject &src, ComAtprotoRepoListMissingBlobs::RecordBlob &dest);
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
void copyContact(const QJsonObject &src, ComAtprotoServerDescribeServer::Contact &dest);
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
void copyIdentity(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::Identity &dest);
void copyAccount(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::Account &dest);
void copyHandle(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::Handle &dest);
void copyMigrate(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::Migrate &dest);
void copyTombstone(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::Tombstone &dest);
void copyInfo(const QJsonObject &src, ComAtprotoSyncSubscribeRepos::Info &dest);
}
// tools.ozone.communication.defs
namespace ToolsOzoneCommunicationDefs {
void copyTemplateView(const QJsonObject &src, ToolsOzoneCommunicationDefs::TemplateView &dest);
}
// tools.ozone.moderation.defs
namespace ToolsOzoneModerationDefs {
void copyModEventTakedown(const QJsonObject &src, ToolsOzoneModerationDefs::ModEventTakedown &dest);
void copyModEventReverseTakedown(const QJsonObject &src,
                                 ToolsOzoneModerationDefs::ModEventReverseTakedown &dest);
void copyModEventComment(const QJsonObject &src, ToolsOzoneModerationDefs::ModEventComment &dest);
void copyModEventReport(const QJsonObject &src, ToolsOzoneModerationDefs::ModEventReport &dest);
void copyModEventLabel(const QJsonObject &src, ToolsOzoneModerationDefs::ModEventLabel &dest);
void copyModEventAcknowledge(const QJsonObject &src,
                             ToolsOzoneModerationDefs::ModEventAcknowledge &dest);
void copyModEventEscalate(const QJsonObject &src, ToolsOzoneModerationDefs::ModEventEscalate &dest);
void copyModEventMute(const QJsonObject &src, ToolsOzoneModerationDefs::ModEventMute &dest);
void copyModEventUnmute(const QJsonObject &src, ToolsOzoneModerationDefs::ModEventUnmute &dest);
void copyModEventMuteReporter(const QJsonObject &src,
                              ToolsOzoneModerationDefs::ModEventMuteReporter &dest);
void copyModEventUnmuteReporter(const QJsonObject &src,
                                ToolsOzoneModerationDefs::ModEventUnmuteReporter &dest);
void copyModEventEmail(const QJsonObject &src, ToolsOzoneModerationDefs::ModEventEmail &dest);
void copyModEventResolveAppeal(const QJsonObject &src,
                               ToolsOzoneModerationDefs::ModEventResolveAppeal &dest);
void copyModEventDivert(const QJsonObject &src, ToolsOzoneModerationDefs::ModEventDivert &dest);
void copyModEventTag(const QJsonObject &src, ToolsOzoneModerationDefs::ModEventTag &dest);
void copyModEventView(const QJsonObject &src, ToolsOzoneModerationDefs::ModEventView &dest);
void copySubjectReviewState(const QJsonValue &src,
                            ToolsOzoneModerationDefs::SubjectReviewState &dest);
void copySubjectStatusView(const QJsonObject &src,
                           ToolsOzoneModerationDefs::SubjectStatusView &dest);
void copyModeration(const QJsonObject &src, ToolsOzoneModerationDefs::Moderation &dest);
void copyRepoView(const QJsonObject &src, ToolsOzoneModerationDefs::RepoView &dest);
void copyRepoViewNotFound(const QJsonObject &src, ToolsOzoneModerationDefs::RepoViewNotFound &dest);
void copyRecordView(const QJsonObject &src, ToolsOzoneModerationDefs::RecordView &dest);
void copyRecordViewNotFound(const QJsonObject &src,
                            ToolsOzoneModerationDefs::RecordViewNotFound &dest);
void copyImageDetails(const QJsonObject &src, ToolsOzoneModerationDefs::ImageDetails &dest);
void copyVideoDetails(const QJsonObject &src, ToolsOzoneModerationDefs::VideoDetails &dest);
void copyBlobView(const QJsonObject &src, ToolsOzoneModerationDefs::BlobView &dest);
void copyModEventViewDetail(const QJsonObject &src,
                            ToolsOzoneModerationDefs::ModEventViewDetail &dest);
void copyModerationDetail(const QJsonObject &src, ToolsOzoneModerationDefs::ModerationDetail &dest);
void copyRepoViewDetail(const QJsonObject &src, ToolsOzoneModerationDefs::RepoViewDetail &dest);
void copyRecordViewDetail(const QJsonObject &src, ToolsOzoneModerationDefs::RecordViewDetail &dest);
}
// tools.ozone.server.getConfig
namespace ToolsOzoneServerGetConfig {
void copyServiceConfig(const QJsonObject &src, ToolsOzoneServerGetConfig::ServiceConfig &dest);
void copyViewerConfig(const QJsonObject &src, ToolsOzoneServerGetConfig::ViewerConfig &dest);
}
// tools.ozone.team.defs
namespace ToolsOzoneTeamDefs {
void copyMember(const QJsonObject &src, ToolsOzoneTeamDefs::Member &dest);
}
// com.whtwnd.blog.defs
namespace ComWhtwndBlogDefs {
void copyBlogEntry(const QJsonObject &src, ComWhtwndBlogDefs::BlogEntry &dest);
void copyComment(const QJsonObject &src, ComWhtwndBlogDefs::Comment &dest);
void copyOgp(const QJsonObject &src, ComWhtwndBlogDefs::Ogp &dest);
void copyBlobMetadata(const QJsonObject &src, ComWhtwndBlogDefs::BlobMetadata &dest);
}
// com.whtwnd.blog.entry
namespace ComWhtwndBlogEntry {
void copyMain(const QJsonObject &src, ComWhtwndBlogEntry::Main &dest);
}
// directory.plc.defs
namespace DirectoryPlcDefs {
void copyDidDocVerificationMethod(const QJsonObject &src,
                                  DirectoryPlcDefs::DidDocVerificationMethod &dest);
void copyDidDocService(const QJsonObject &src, DirectoryPlcDefs::DidDocService &dest);
void copyDidDoc(const QJsonObject &src, DirectoryPlcDefs::DidDoc &dest);
void copyPlcLogAtprotoPds(const QJsonObject &src, DirectoryPlcDefs::PlcLogAtprotoPds &dest);
void copyPlcLogService(const QJsonObject &src, DirectoryPlcDefs::PlcLogService &dest);
void copyPlcLogVerificationMethods(const QJsonObject &src,
                                   DirectoryPlcDefs::PlcLogVerificationMethods &dest);
void copyPlc_operation(const QJsonObject &src, DirectoryPlcDefs::Plc_operation &dest);
void copyPlc_tombstone(const QJsonObject &src, DirectoryPlcDefs::Plc_tombstone &dest);
void copyCreate(const QJsonObject &src, DirectoryPlcDefs::Create &dest);
void copyPlcAuditLogDetail(const QJsonObject &src, DirectoryPlcDefs::PlcAuditLogDetail &dest);
void copyPlcAuditLog(const QJsonArray &src, DirectoryPlcDefs::PlcAuditLog &dest);
}
// oauth.defs
namespace OauthDefs {
void copyPushedAuthorizationResponse(const QJsonObject &src,
                                     OauthDefs::PushedAuthorizationResponse &dest);
void copyTokenResponse(const QJsonObject &src, OauthDefs::TokenResponse &dest);
}
// wellKnown.defs
namespace WellKnownDefs {
void copyResourceMetadata(const QJsonObject &src, WellKnownDefs::ResourceMetadata &dest);
void copyServerMetadata(const QJsonObject &src, WellKnownDefs::ServerMetadata &dest);
}

}

#endif // LEXICONS_FUNC_H
