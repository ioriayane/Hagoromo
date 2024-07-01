import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.timelinelistmodel 1.0
import tech.relog.hagoromo.notificationlistmodel 1.0
import tech.relog.hagoromo.columnlistmodel 1.0
import tech.relog.hagoromo.searchpostlistmodel 1.0
import tech.relog.hagoromo.searchprofilelistmodel 1.0
import tech.relog.hagoromo.customfeedlistmodel 1.0
import tech.relog.hagoromo.authorfeedlistmodel 1.0
import tech.relog.hagoromo.anyprofilelistmodel 1.0
import tech.relog.hagoromo.listfeedlistmodel 1.0
import tech.relog.hagoromo.chatlistmodel 1.0
import tech.relog.hagoromo.chatmessagelistmodel 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"
import "../compat"

ColumnLayout {
    id: columnView
    spacing: 0

    property string columnKey: ""
    property int componentType: 0
    property bool selected: false

    property alias settings: settings
    property alias account: account
    property string hoveredLink: ""

    signal requestReply(string account_uuid,
                        string cid, string uri,
                        string reply_root_cid, string reply_root_uri,
                        string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestQuote(string account_uuid,
                        string cid, string uri,
                        string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestMention(string account_uuid, string handle)
    signal requestMessage(string account_uuid, string did, string current_column_key)
    signal requestViewAuthorFeed(string account_uuid, string did, string handle)
    signal requestViewImages(int index, var paths, var alts)
    signal requestViewFeedGenerator(string account_uuid, string name, string uri)
    signal requestViewSearchPosts(string account_uuid, string text, string current_column_key)
    signal requestViewListFeed(string account_uuid, string uri, string name)
    signal requestReportPost(string account_uuid, string uri, string cid)
    signal requestReportAccount(string account_uuid, string did)
    signal requestReportMessage(string account_uuid, string did, string convo_id, string message_id)
    signal requestAddRemoveFromLists(string account_uuid, string did)
    signal requestAddMutedWord(string account_uuid, string text)
    signal requestEditProfile(string account_uuid, string did, string avatar, string banner, string display_name, string description)
    signal requestEditList(string account_uuid, string uri, string avatar, string name, string description)
    signal requestUpdateThreadGate(string account_uuid, string uri, string threadgate_uri, string type, var rules, var callback)

    signal requestMoveToLeft(string key)
    signal requestMoveToRight(string key)
    signal requestRemove(string key)
    signal requestDisplayOfColumnSetting(string key)

    signal errorOccured(string account_uuid, string code, string message)

    Shortcut {
        enabled: columnView.selected
        context: Qt.ApplicationShortcut
        sequence: StandardKey.MoveToPreviousPage
        onActivated: console.log("PageUp:" + columnView.columnKey)
    }
    Shortcut {
        enabled: columnView.selected
        context: Qt.ApplicationShortcut
        sequence: StandardKey.MoveToNextPage
        onActivated: console.log("PageDown:" + columnView.columnKey)
    }

    ColumnSettings {
        id: settings
    }
    Account {
        id: account
    }
    SystemTool {
        id: systemTool
    }

    Component {
        id: timelineComponent
        TimelineView {
            model: TimelineListModel {
                autoLoading: settings.autoLoading
                loadingInterval: settings.loadingInterval
                displayInterval: settings.displayInterval
                visibleReplyToUnfollowedUsers: settings.visibleReplyToUnfollowedUsers
                visibleRepostOfOwn: settings.visibleRepostOfOwn
                visibleRepostOfFollowingUsers: settings.visibleRepostOfFollowingUsers
                visibleRepostOfUnfollowingUsers: settings.visibleRepostOfUnfollowingUsers
                visibleRepostOfMine: settings.visibleRepostOfMine
                visibleRepostByMe: settings.visibleRepostByMe
                visibleContainingMutedWord: false   // ミュートワードを含むときは完全非表示

                onErrorOccured: (code, message) => columnView.errorOccured(columnView.account.uuid, code, message)
            }
            accountDid: account.did
            imageLayoutType: settings.imageLayoutType

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(columnView.account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(columnView.account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }

            onRequestViewImages: (index, paths, alts) => columnView.requestViewImages(index, paths, alts)

            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewFeedGenerator: (name, uri) => columnView.requestViewFeedGenerator(account.uuid, name, uri)
            onRequestViewListFeed: (uri, name) => columnView.requestViewListFeed(account.uuid, uri, name)
            onRequestViewLikedBy: (uri) => columnStackView.push(likesProfilesComponent, { "targetUri": uri })
            onRequestViewRepostedBy: (uri) => columnStackView.push(repostsProfilesComponent, { "targetUri": uri })
            onRequestViewSearchPosts: (text) => columnView.requestViewSearchPosts(account.uuid, text, columnView.columnKey)
            onRequestReportPost: (uri, cid) => columnView.requestReportPost(account.uuid, uri, cid)
            onRequestAddMutedWord: (text) => columnView.requestAddMutedWord(account.uuid, text)
            onRequestUpdateThreadGate: (uri, threadgate_uri, type, rules, callback) => columnView.requestUpdateThreadGate(account.uuid, uri, threadgate_uri, type, rules, callback)
            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }
    Component {
        id: listNotificationComponent
        NotificationListView {
            model: NotificationListModel {
                autoLoading: settings.autoLoading
                loadingInterval: settings.loadingInterval
                displayInterval: settings.displayInterval
                visibleLike: settings.visibleLike
                visibleRepost: settings.visibleRepost
                visibleFollow: settings.visibleFollow
                visibleMention: settings.visibleMention
                visibleReply: settings.visibleReply
                visibleQuote: settings.visibleQuote
                visibleContainingMutedWord: true   // 通知はミュートワードを含んでらたたんで表示
                updateSeenNotification: settings.updateSeenNotification
                aggregateReactions: settings.aggregateReactions

                onErrorOccured: (code, message) => columnView.errorOccured(columnView.account.uuid, code, message)
            }
            imageLayoutType: settings.imageLayoutType

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)
            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }
            onRequestViewImages: (index, paths, alts) => columnView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewFeedGenerator: (name, uri) => columnView.requestViewFeedGenerator(account.uuid, name, uri)
            onRequestViewListFeed: (uri, name) => columnView.requestViewListFeed(account.uuid, uri, name)
            onRequestViewLikedBy: (uri) => columnStackView.push(likesProfilesComponent, { "targetUri": uri })
            onRequestViewRepostedBy: (uri) => columnStackView.push(repostsProfilesComponent, { "targetUri": uri })
            onRequestViewSearchPosts: (text) => columnView.requestViewSearchPosts(account.uuid, text, columnView.columnKey)
            onRequestReportPost: (uri, cid) => columnView.requestReportPost(account.uuid, uri, cid)
            onRequestAddMutedWord: (text) => columnView.requestAddMutedWord(account.uuid, text)

            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }
    Component {
        id: postThreadComponent
        PostThreadView {
            accountDid: account.did
            imageLayoutType: settings.imageLayoutType

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }
            onRequestViewImages: (index, paths, alts) => columnView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewFeedGenerator: (name, uri) => columnView.requestViewFeedGenerator(account.uuid, name, uri)
            onRequestViewListFeed: (uri, name) => columnView.requestViewListFeed(account.uuid, uri, name)
            onRequestViewLikedBy: (uri) => columnStackView.push(likesProfilesComponent, { "targetUri": uri })
            onRequestViewRepostedBy: (uri) => columnStackView.push(repostsProfilesComponent, { "targetUri": uri })
            onRequestViewSearchPosts: (text) => columnView.requestViewSearchPosts(account.uuid, text, columnView.columnKey)
            onRequestReportPost: (uri, cid) => columnView.requestReportPost(account.uuid, uri, cid)
            onRequestAddMutedWord: (text) => columnView.requestAddMutedWord(account.uuid, text)
            onRequestUpdateThreadGate: (uri, threadgate_uri, type, rules, callback) => columnView.requestUpdateThreadGate(account.uuid, uri, threadgate_uri, type, rules, callback)

            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
            onErrorOccured: (code, message) => columnView.errorOccured(columnView.account.uuid, code, message)
            onBack: {
                if(!columnStackView.empty){
                    columnStackView.pop()
                }
            }
        }
    }
    Component {
        id: profileComponent
        ProfileView {
            imageLayoutType: settings.imageLayoutType

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)
            onRequestMention: (handle) =>
                              columnView.requestMention(account.uuid, handle)
            onRequestMessage: (did) => columnView.requestMessage(account.uuid, did, columnView.columnKey)
            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }
            onRequestViewAuthorFeed: (did, handle) =>
                                     columnView.requestViewAuthorFeed(account.uuid, did, handle)

            onRequestViewImages: (index, paths, alts) => columnView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewFeedGenerator: (name, uri) => columnView.requestViewFeedGenerator(account.uuid, name, uri)
            onRequestViewListFeed: (uri, name) => columnView.requestViewListFeed(account.uuid, uri, name)
            onRequestViewLikedBy: (uri) => columnStackView.push(likesProfilesComponent, { "targetUri": uri })
            onRequestViewRepostedBy: (uri) => columnStackView.push(repostsProfilesComponent, { "targetUri": uri })
            onRequestViewSearchPosts: (text) => columnView.requestViewSearchPosts(account.uuid, text, columnView.columnKey)
            onRequestViewListDetail: (uri) => columnStackView.push(listDetailComponent, { "listUri": uri })
            onRequestReportPost: (uri, cid) => columnView.requestReportPost(account.uuid, uri, cid)
            onRequestReportAccount: (did) => columnView.requestReportAccount(account.uuid, did)
            onRequestAddMutedWord: (text) => columnView.requestAddMutedWord(account.uuid, text)
            onRequestUpdateThreadGate: (uri, threadgate_uri, type, rules, callback) => columnView.requestUpdateThreadGate(account.uuid, uri, threadgate_uri, type, rules, callback)
            onRequestAddRemoveFromLists: (did) => columnView.requestAddRemoveFromLists(account.uuid, did)
            onRequestEditProfile: (did, avatar, banner, display_name, description) => {
                                      columnView.requestEditProfile(account.uuid, did, avatar, banner, display_name, description)
                                  }
            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink

            onErrorOccured: (code, message) => columnView.errorOccured(columnView.account.uuid, code, message)
            onBack: {
                if(!columnStackView.empty){
                    columnStackView.pop()
                }
            }
        }
    }
    Component {
        id: searchPostsComponent
        TimelineView {
            model: SearchPostListModel {
                autoLoading: settings.autoLoading
                loadingInterval: settings.loadingInterval
                displayInterval: settings.displayInterval
                text: settings.columnValue

                onErrorOccured: (code, message) => columnView.errorOccured(columnView.account.uuid, code, message)
            }
            accountDid: account.did
            imageLayoutType: settings.imageLayoutType

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }

            onRequestViewImages: (index, paths, alts) => columnView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewFeedGenerator: (name, uri) => columnView.requestViewFeedGenerator(account.uuid, name, uri)
            onRequestViewListFeed: (uri, name) => columnView.requestViewListFeed(account.uuid, uri, name)
            onRequestViewLikedBy: (uri) => columnStackView.push(likesProfilesComponent, { "targetUri": uri })
            onRequestViewRepostedBy: (uri) => columnStackView.push(repostsProfilesComponent, { "targetUri": uri })
            onRequestViewSearchPosts: (text) => columnView.requestViewSearchPosts(account.uuid, text, columnView.columnKey)
            onRequestReportPost: (uri, cid) => columnView.requestReportPost(account.uuid, uri, cid)
            onRequestAddMutedWord: (text) => columnView.requestAddMutedWord(account.uuid, text)
            onRequestUpdateThreadGate: (uri, threadgate_uri, type, rules, callback) => columnView.requestUpdateThreadGate(account.uuid, uri, threadgate_uri, type, rules, callback)

            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }
    Component {
        id: searchProfilesComponent
        ProfileListView {
            accountDid: account.did
            unfollowAndRemove: false
            model: SearchProfileListModel {
                autoLoading: settings.autoLoading
                text: settings.columnValue

                onErrorOccured: (code, message) => columnView.errorOccured(columnView.account.uuid, code, message)
            }
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }
    Component {
        id: customComponent
        TimelineView {
            model: CustomFeedListModel {
                autoLoading: settings.autoLoading
                loadingInterval: settings.loadingInterval
                displayInterval: settings.displayInterval
                uri: settings.columnValue
                visibleContainingMutedWord: false   // ミュートワードを含むときは完全非表示
                onSavingChanged: {
                    saveFeedMenuItem.saving = saving
                }
                onErrorOccured: (code, message) => columnView.errorOccured(columnView.account.uuid, code, message)
            }
            accountDid: account.did
            imageLayoutType: settings.imageLayoutType

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }

            onRequestViewImages: (index, paths, alts) => columnView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewFeedGenerator: (name, uri) => columnView.requestViewFeedGenerator(account.uuid, name, uri)
            onRequestViewListFeed: (uri, name) => columnView.requestViewListFeed(account.uuid, uri, name)
            onRequestViewLikedBy: (uri) => columnStackView.push(likesProfilesComponent, { "targetUri": uri })
            onRequestViewRepostedBy: (uri) => columnStackView.push(repostsProfilesComponent, { "targetUri": uri })
            onRequestViewSearchPosts: (text) => columnView.requestViewSearchPosts(account.uuid, text, columnView.columnKey)
            onRequestReportPost: (uri, cid) => columnView.requestReportPost(account.uuid, uri, cid)
            onRequestAddMutedWord: (text) => columnView.requestAddMutedWord(account.uuid, text)
            onRequestUpdateThreadGate: (uri, threadgate_uri, type, rules, callback) => columnView.requestUpdateThreadGate(account.uuid, uri, threadgate_uri, type, rules, callback)

            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }

    Component {
        id: authorFeedComponent
        TimelineView {
            model: AuthorFeedListModel {
                autoLoading: settings.autoLoading
                loadingInterval: settings.loadingInterval
                displayInterval: 0
                authorDid: settings.columnValue

                onErrorOccured: (code, message) => columnView.errorOccured(columnView.account.uuid, code, message)
            }
            accountDid: account.did
            imageLayoutType: settings.imageLayoutType

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }

            onRequestViewImages: (index, paths, alts) => columnView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewFeedGenerator: (name, uri) => columnView.requestViewFeedGenerator(account.uuid, name, uri)
            onRequestViewListFeed: (uri, name) => columnView.requestViewListFeed(account.uuid, uri, name)
            onRequestViewLikedBy: (uri) => columnStackView.push(likesProfilesComponent, { "targetUri": uri })
            onRequestViewRepostedBy: (uri) => columnStackView.push(repostsProfilesComponent, { "targetUri": uri })
            onRequestViewSearchPosts: (text) => columnView.requestViewSearchPosts(account.uuid, text, columnView.columnKey)
            onRequestReportPost: (uri, cid) => columnView.requestReportPost(account.uuid, uri, cid)
            onRequestAddMutedWord: (text) => columnView.requestAddMutedWord(account.uuid, text)
            onRequestUpdateThreadGate: (uri, threadgate_uri, type, rules, callback) => columnView.requestUpdateThreadGate(account.uuid, uri, threadgate_uri, type, rules, callback)

            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }
    Component {
        id: likesProfilesComponent
        AnyProfileListView {
            accountDid: account.did
            autoLoading: settings.autoLoading
            type: AnyProfileListModel.Like

            onErrorOccured: (code, message) => columnView.errorOccured(columnView.account.uuid, code, message)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
            onBack: {
                if(!columnStackView.empty){
                    columnStackView.pop()
                }
            }
        }
    }
    Component {
        id: repostsProfilesComponent
        AnyProfileListView {
            accountDid: account.did
            autoLoading: settings.autoLoading
            type: AnyProfileListModel.Repost

            onErrorOccured: (code, message) => columnView.errorOccured(columnView.account.uuid, code, message)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
            onBack: {
                if(!columnStackView.empty){
                    columnStackView.pop()
                }
            }
        }
    }
    Component {
        id: listDetailComponent
        ListDetailView {
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewListFeed: (uri, name) => columnView.requestViewListFeed(account.uuid, uri, name)
            onRequestEditList: (uri, avatar, name, description) => columnView.requestEditList(account.uuid, uri, avatar, name, description)

            onErrorOccured: (code, message) => columnView.errorOccured(columnView.account.uuid, code, message)
            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink

            onBack: {
                if(!columnStackView.empty){
                    columnStackView.pop()
                }
            }
        }
    }
    Component {
        id: listFeedComponent
        TimelineView {
            model: ListFeedListModel {
                autoLoading: settings.autoLoading
                loadingInterval: settings.loadingInterval
                uri: settings.columnValue
                visibleContainingMutedWord: false   // ミュートワードを含むときは完全非表示

                onErrorOccured: (code, message) => columnView.errorOccured(columnView.account.uuid, code, message)
            }
            accountDid: account.did
            imageLayoutType: settings.imageLayoutType

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(columnView.account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(columnView.account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }

            onRequestViewImages: (index, paths, alts) => columnView.requestViewImages(index, paths, alts)

            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewFeedGenerator: (name, uri) => columnView.requestViewFeedGenerator(account.uuid, name, uri)
            onRequestViewLikedBy: (uri) => columnStackView.push(likesProfilesComponent, { "targetUri": uri })
            onRequestViewRepostedBy: (uri) => columnStackView.push(repostsProfilesComponent, { "targetUri": uri })
            onRequestViewSearchPosts: (text) => columnView.requestViewSearchPosts(account.uuid, text, columnView.columnKey)
            onRequestReportPost: (uri, cid) => columnView.requestReportPost(account.uuid, uri, cid)
            onRequestAddMutedWord: (text) => columnView.requestAddMutedWord(account.uuid, text)
            onRequestUpdateThreadGate: (uri, threadgate_uri, type, rules, callback) => columnView.requestUpdateThreadGate(account.uuid, uri, threadgate_uri, type, rules, callback)

            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }

    Component {
        id: chatListComponent
        ChatListView {
            onRequestViewChatMessage: (convo_id, dids) => {
                                          console.log("onRequestViewChatMessage:" + convo_id + ", " + dids)
                                          settings.columnValue = convo_id
                                          settings.columnValueList = dids
                                          columnStackView.push(chatMessageListComponent)
                                      }
            onErrorOccured: (code, message) => columnView.errorOccured(columnView.account.uuid, code, message)
        }
    }
    Component {
        id: chatMessageListComponent
        ChatMessageListView {
            id: chatMesssageListView
            accountDid: account.did
            listView.model: ChatMessageListModel {
                convoId: settings.columnValue
                memberDids: settings.columnValueList
                autoLoading: true
                loadingInterval: 2000
                onErrorOccured: (code, message) => {
                                    if(code === "InvalidToken" && message === "Bad token scope"){
                                        chatMesssageListView.errorMessageOnChatMessageList.visible = true
                                    }else{
                                        columnView.errorOccured(columnView.account.uuid, code, message)
                                    }
                                }
                onFinishSent: (success) => chatMesssageListView.finishSent(success)
            }

            onRequestReportMessage: (did, convo_id, message_id) => columnView.requestReportMessage(account.uuid, did, convo_id, message_id)
            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }
            onRequestViewImages: (index, paths, alts) => columnView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewSearchPosts: (text) => columnView.requestViewSearchPosts(account.uuid, text, columnView.columnKey)
            onRequestAddMutedWord: (text) => columnView.requestAddMutedWord(account.uuid, text)
            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }

    function load(){
        console.log("ColumnLayout:componentType=" + componentType)
        if(componentType === 0){
            columnStackView.push(timelineComponent)
            componentTypeLabel.addText = ""
        }else if(componentType === 1){
            columnStackView.push(listNotificationComponent)
            componentTypeLabel.addText = ""
        }else if(componentType === 2){
            columnStackView.push(searchPostsComponent)
            componentTypeLabel.addText = " : " + settings.columnValue
        }else if(componentType === 3){
            columnStackView.push(searchProfilesComponent)
            componentTypeLabel.addText = " : " + settings.columnValue
        }else if(componentType === 4){
            columnStackView.push(customComponent)
            componentTypeLabel.addText = " : " + settings.columnName
        }else if(componentType === 5){
            columnStackView.push(authorFeedComponent)
            componentTypeLabel.addText = " : " + settings.columnName
        }else if(componentType === 6){
            columnStackView.push(listFeedComponent)
            componentTypeLabel.addText = " : " + settings.columnName
        }else if(componentType === 7){
            columnStackView.push(chatListComponent)
            componentTypeLabel.addText = ""
        }else if(componentType === 8){
            columnStackView.push(chatMessageListComponent)
            componentTypeLabel.addText = ""
        }else{
            columnStackView.push(timelineComponent)
            componentTypeLabel.addText = ""
        }
    }

    function reflect(){
        // StackViewに積まれているViewに反映
        for(var i=0; i<columnStackView.depth; i++){
            console.log("Reflect : " + i + ", " + account.handle)
            var item = columnStackView.get(i)
            item.model.setAccount(account.service,
                                  account.did,
                                  account.handle,
                                  account.email,
                                  account.accessJwt,
                                  account.refreshJwt)
        }
    }

    ClickableFrame {
        id: profileFrame
        Layout.fillWidth: true
        Layout.topMargin: 1
        leftPadding: 0
        topPadding: 0
        rightPadding: 10
        bottomPadding: 0

        borderColor: columnView.selected ? Material.color(Material.BlueGrey) : Material.color(Material.Grey, Material.Shade600)

        onClicked: (mouse) => {
                       if(columnStackView.currentItem.listView){
                           columnStackView.currentItem.listView.positionViewAtBeginning()
                       }
                   }

        RowLayout {
            id: headerLayout
            anchors.fill: parent
            spacing: 0
            IconButton {
                id: leftIconButton
                Layout.preferredWidth: AdjustedValues.b30
                Layout.preferredHeight: AdjustedValues.b30
                visible: columnStackView.depth > 1
                flat: true
                iconSource: "../images/arrow_left_double.png"
                onClicked: columnStackView.pop(null)
            }

            AvatarImage {
                id: avatarImage
                Layout.leftMargin: 10
                Layout.preferredWidth: AdjustedValues.i24
                Layout.preferredHeight: AdjustedValues.i24
                source: account.avatar
                onClicked: columnStackView.push(profileComponent, { "userDid": account.did })
            }

            ColumnLayout {
                spacing: 0
                Layout.leftMargin: 5
                Layout.topMargin: 5
                Layout.bottomMargin: 5
                Layout.preferredWidth: accountInfoLabel.contentWidth
                Layout.maximumWidth: columnView.width - profileFrame.rightPadding -
                                     leftIconButton.Layout.preferredWidth -
                                     avatarImage.Layout.preferredWidth - avatarImage.Layout.leftMargin -
                                     autoIconImage.Layout.preferredWidth - autoIconImage.Layout.rightMargin -
                                     settingButton.Layout.preferredWidth - 5
                Label {
                    id: componentTypeLabel
                    elide: Text.ElideRight
                    font.pointSize: AdjustedValues.f10
                    text: baseText[componentType] + addText
                    property var baseText: [
                        qsTr("Home"),
                        qsTr("Notifications"),
                        qsTr("Search posts"),
                        qsTr("Search users"),
                        qsTr("Feed"),
                        qsTr("User"),
                        qsTr("List"),
                        qsTr("Chat"),
                        qsTr("Chat"),
                        qsTr("Unknown")
                    ]
                    property string addText: ""
                }
                Label {
                    id: accountInfoLabel
                    text: "@" + account.handle + " - " + account.service
                    font.pointSize: AdjustedValues.f8
                    elide: Text.ElideRight
                    color: Material.color(Material.Grey)
                }
            }
            Item {
                Layout.fillWidth: true
                height: 1
            }
            Image {
                id: autoIconImage
                Layout.preferredWidth: AdjustedValues.i16
                Layout.preferredHeight: AdjustedValues.i16
                Layout.rightMargin: 3
                Layout.alignment: Qt.AlignVCenter
                source: "../images/auto.png"
                layer.enabled: true
                layer.effect: ColorOverlayC {
                    color: settings.autoLoading ? Material.accentColor : Material.color(Material.Grey)
                }
            }
            IconButton {
                id: settingButton
                Layout.preferredWidth: AdjustedValues.b30
                Layout.preferredHeight: AdjustedValues.b24
                iconSource: "../images/settings.png"
                iconSize: AdjustedValues.i16
                onClicked: popup.open()
                Menu {
                    id: popup
                    MenuItem {
                        icon.source: "../images/arrow_back.png"
                        text: qsTr("Move to left")
                        onTriggered: requestMoveToLeft(columnKey)
                    }
                    MenuItem {
                        icon.source: "../images/arrow_forward.png"
                        text: qsTr("Move to right")
                        onTriggered: requestMoveToRight(columnKey)
                    }
                    MenuItem {
                        icon.source: "../images/delete.png"
                        text: qsTr("Delete column")
                        onTriggered: requestRemove(columnKey)
                    }
                    MenuSeparator {}
                    Menu {
                        id: feedMenu
                        title: qsTr("Feed")
                        enabled: (componentType === 4) && columnStackView.depth == 1
                        MenuItem {
                            text: qsTr("Copy url")
                            icon.source: "../images/copy.png"
                            onTriggered: {
                                if(componentType === 4){
                                    systemTool.copyToClipboard(columnStackView.get(0).model.getOfficialUrl())
                                }
                            }
                        }
                        MenuItem {
                            text: qsTr("Open in Official")
                            icon.source: "../images/open_in_other.png"
                            onTriggered: {
                                if(componentType === 4){
                                    Qt.openUrlExternally(columnStackView.get(0).model.getOfficialUrl())
                                }
                            }
                        }
                        MenuItem {
                            id: saveFeedMenuItem
                            icon.source: "../images/bookmark_add.png"
                            text: saving ? qsTr("Drop") : qsTr("Save")
                            property bool saving: false
                            onTriggered: {
                                if(componentType === 4){
                                    if(saving){
                                        console.log("drop bookmark")
                                        columnStackView.get(0).model.removeGenerator()
                                    }else{
                                        console.log("save bookmark")
                                        columnStackView.get(0).model.saveGenerator()
                                    }
                                }
                            }
                        }
                    }
                    MenuSeparator {}
                    MenuItem {
                        icon.source: "../images/settings.png"
                        text: qsTr("Settings")
                        onTriggered: requestDisplayOfColumnSetting(columnKey)
                    }
                }
            }
        }
    }

    StackView {
        id: columnStackView
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true

        onCurrentItemChanged: {
            if(currentItem.model === undefined){
                return
            }
            if(currentItem.model.rowCount() > 0){
                if(currentItem.resume){
                    currentItem.resume()
                }
                return
            }
            currentItem.model.setAccount(account.service,
                                         account.did,
                                         account.handle,
                                         account.email,
                                         account.accessJwt,
                                         account.refreshJwt)
            currentItem.model.getLatest()
        }
    }
}
