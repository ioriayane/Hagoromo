import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.userprofile 1.0
import tech.relog.hagoromo.authorfeedlistmodel 1.0
import tech.relog.hagoromo.anyfeedlistmodel 1.0
import tech.relog.hagoromo.recordoperator 1.0
import tech.relog.hagoromo.followslistmodel 1.0
import tech.relog.hagoromo.followerslistmodel 1.0
import tech.relog.hagoromo.actorfeedgeneratorlistmodel 1.0
import tech.relog.hagoromo.listslistmodel 1.0
import tech.relog.hagoromo.blog.blogentrylistmodel 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"
import "../compat"

ColumnLayout {
    id: profileView
    Layout.fillWidth: true
    Layout.fillHeight: true
    spacing: 0

    property string hoveredLink: ""
    property int imageLayoutType: 1

    property alias model: relayObject

    property string userDid: ""     // 表示するアカウント
    property string accountDid: ""  // 認証しているアカウント

    property bool autoHideDetailMode: true

    signal requestReply(string cid, string uri,
                        string reply_root_cid, string reply_root_uri,
                        string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestQuote(string cid, string uri, string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestMention(string handle)
    signal requestMessage(string did)
    signal requestViewThread(string uri)
    signal requestViewQuotes(string uri)
    signal requestViewImages(int index, var paths, var alts)
    signal requestViewProfile(string did)
    signal requestViewKnownFollowers(string did)
    signal requestViewFeedGenerator(string name, string uri)
    signal requestViewListFeed(string uri, string name)
    signal requestViewAuthorFeed(string did, string handle)
    signal requestViewLikedBy(string uri)
    signal requestViewRepostedBy(string uri)
    signal requestViewSearchPosts(string text)
    signal requestViewListDetail(string uri)
    signal requestUpdateThreadGate(string uri, string threadgate_uri, string type, var rules, var callback)
    signal requestReportPost(string uri, string cid)
    signal requestReportAccount(string did)
    signal requestAddRemoveFromLists(string did)
    signal requestAddMutedWord(string text)
    signal requestEditProfile(string did, string avatar, string banner, string display_name, string description)

    signal errorOccured(string code, string message)
    signal back()

    states: [
        State {
            // 通信中
            when: recordOperator.running || userProfile.running
            PropertyChanges { target: editButton; iconText: "   " }
            PropertyChanges { target: editButton; enabled: false }
        },
        State {
            // 認証しているアカウントを表示しているとき
            when: userDid === accountDid
            PropertyChanges { target: editButton; visible: true }
            PropertyChanges { target: editButton; iconText: qsTr("Edit Profile") }
            PropertyChanges { target: editButton; onClicked: {
                    profileView.requestEditProfile(accountDid,
                                                   userProfile.avatar,
                                                   userProfile.banner,
                                                   userProfile.displayName,
                                                   userProfile.description)
                } }
        },
        State {
            // フォローしている
            when: userProfile.following
            PropertyChanges { target: editButton; iconText: qsTr("Following") }
            PropertyChanges { target: editButton; highlighted: true }
            PropertyChanges { target: editButton; onClicked: {
                    userProfile.following = false
                    recordOperator.deleteFollow(userProfile.followingUri)
                } }
        },
        State {
            // フォローしていない
            when: !userProfile.following
            PropertyChanges { target: editButton; iconText: qsTr("Follow") }
            PropertyChanges { target: editButton; onClicked: {
                    userProfile.following = true
                    recordOperator.follow(profileView.userDid)
                } }
        }
    ]

    function getOfficialUrl(did) {
        if(did.length === 0){
            return ""
        }
        return "https://bsky.app/profile/" + did
    }

    function openInOhters(handle) {
        if(handle.length === 0){
            return
        }
        Qt.openUrlExternally(getOfficialUrl(handle))
    }

    QtObject {
        id: relayObject
        function rowCount() {
            return userProfile.handle.length;
        }
        function setAccount(uuid) {
            recordOperator.setAccount(uuid)
            userProfile.setAccount(uuid)
            authorFeedListModel.setAccount(uuid)
            authorBlogEntryListModel.setAccount(uuid)
            repostFeedListModel.setAccount(uuid)
            likesFeedListModel.setAccount(uuid)
            authorMediaFeedListModel.setAccount(uuid)
            authorVideoFeedListModel.setAccount(uuid)
            actorFeedGeneratorListModel.setAccount(uuid)
            listsListModel.setAccount(uuid)
            followsListModel.setAccount(uuid)
            followersListModel.setAccount(uuid)
            accountDid = authorFeedListModel.did
        }
        function getLatest() {
            userProfile.getProfile(userDid)
            tabBar.currentIndex = 0
        }
    }

    RecordOperator {
        id: recordOperator
        onFinished: (success) => userProfile.getProfile(userDid)
    }

    UserProfile {
        id: userProfile
        onRunningChanged: {
            if(running === false){
                swipView.getLatest()
            }
        }
    }

    SystemTool {
        id: systemTool
    }

    Frame {
        Layout.fillWidth: true
        leftPadding: 0
        topPadding: 0
        rightPadding: 10
        bottomPadding: 0

        RowLayout {
            IconButton {
                Layout.preferredWidth: AdjustedValues.b30
                Layout.preferredHeight: AdjustedValues.b30
                flat: true
                iconSource: "../images/arrow_left_single.png"
                onClicked: profileView.back()
            }
            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 10
                font.pointSize: AdjustedValues.f10
                text: qsTr("Profile")
            }
        }
    }

    ColumnLayout {
        id: userProfileColumnLayout
        Layout.fillWidth: true
        Layout.topMargin: 0
        Layout.leftMargin: 0
        Layout.rightMargin: 0
        // Layout.bottomMargin: 5
        clip: true
        spacing: 0

        function viewChange(top, is_manual){
            console.log("view change : " + top + ", " + profileView.autoHideDetailMode +
                        ", d_h=" + bannerImage.height +
                        "," + bannerImage.paintedHeight +
                        "," + bannerImage.Layout.preferredHeight)
            if(!profileView.autoHideDetailMode && !is_manual){
                return
            }
            if(top){
                bannerImage.Layout.preferredHeight = bannerImage.isReady ? 80 : 0
                descriptionLabel.Layout.preferredHeight = descriptionLabel.contentHeight
                serviceEndpointLayout.visible = true
                registrationDateLayout.visible = true
                knownFollowersInfo.visible = (profileView.userDid !== profileView.accountDid) // true
            }else{
                bannerImage.Layout.preferredHeight = 0
                descriptionLabel.Layout.preferredHeight = 0
                serviceEndpointLayout.visible = false
                registrationDateLayout.visible = false
                knownFollowersInfo.visible = false
            }
        }

        ImageWithIndicator {
            id: bannerImage
            Layout.preferredWidth: profileView.width
            Layout.preferredHeight: 0
            fillMode: Image.PreserveAspectCrop
            source: userProfile.banner

            onIsReadyChanged: Layout.preferredHeight = isReady ? 80 : 0
            Behavior on Layout.preferredHeight {
                NumberAnimation { duration: 500 }
            }

            ColumnLayout{
                anchors.top: bannerImage.bottom
                anchors.right: bannerImage.right
                RowLayout {
                    Label {
                        visible: userProfile.followedBy
                        font.pointSize: AdjustedValues.f8
                        color: Material.accentColor
                        text: qsTr("Follows you")
                    }
                    IconButton {
                        id: editButton
                        Layout.preferredHeight: AdjustedValues.b24
                        iconText: "   "
                        BusyIndicator {
                            anchors.fill: parent
                            visible: recordOperator.running || userProfile.running
                        }
                    }
                }
                IconButton {
                    Layout.preferredHeight: AdjustedValues.b24
                    Layout.alignment: Qt.AlignRight
                    iconSource: serviceEndpointLayout.visible ?
                                    "../images/expand_content_less.png" :
                                    "../images/expand_content_more.png"
                    iconText: ""
                    visible: !profileView.autoHideDetailMode
                    onClicked: userProfileColumnLayout.viewChange(!serviceEndpointLayout.visible, true)
                }
            }
        }
        RowLayout {
            Layout.topMargin: 5
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            AvatarImage {
                id: avatarImage
                Layout.preferredWidth: AdjustedValues.i48
                Layout.preferredHeight: AdjustedValues.i48
                Layout.alignment: Qt.AlignTop
                Layout.rowSpan: 2
                source: userProfile.avatar

                LiveIconCover {
                    anchors.fill: parent
                    visible: userProfile.liveIsActive
                }
                ExternalLinkCardPopup {
                    enabled: userProfile.liveIsActive
                    layoutMaximumWidth: profileView.width
                    linkUri: userProfile.liveLinkUri
                    linkTitle: userProfile.liveLinkTitle
                    linkDescription: userProfile.liveLinkDescription
                    linkThumb: userProfile.liveLinkThumb
                    linkExpiresAt: userProfile.liveExpiresAt
                    onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
                }
            }
            ColumnLayout {
                Label {
                    Layout.preferredWidth: profileView.width - avatarImage.width - parent.columnSpacing
                    font.pointSize: AdjustedValues.f12
                    elide: Text.ElideRight
                    text: userProfile.displayName
                }
                RowLayout {
                    Layout.preferredWidth: profileView.width - avatarImage.width - parent.columnSpacing
                    Label {
                        id: handleLabel
                        elide: Text.ElideRight
                        font.pointSize: AdjustedValues.f8
                        color: Material.color(Material.Grey)
                        text: "@" + userProfile.handle
                        visible: userProfile.handle.length > 0

                        HandleHistoryPopup {
                            id: handleHistoryPopup
                            model: userProfile.handleHistory
                        }
                    }
                    Image {
                        Layout.preferredWidth: handleLabel.height
                        Layout.preferredHeight: handleLabel.height
                        visible: (userProfile.verificationState !== "none")
                        source: userProfile.verificationState === "verifier" ?
                                    "../images/verifier.png" : "../images/verified.png"
                        layer.enabled: true
                        layer.effect: ColorOverlayC {
                            color: Material.color(Material.Blue)
                        }
                        VerificationInfoPopup {
                            model: userProfile.verifierList
                            verificationState: userProfile.verificationState
                            displayName: userProfile.displayName
                            layoutMaximumWidth: profileView.width
                        }
                    }
                }
                RowLayout {
                    spacing: 3
                    Label {
                        font.pointSize: AdjustedValues.f8
                        font.bold: true
                        text: userProfile.followsCount
                    }
                    Label {
                        font.pointSize: AdjustedValues.f8
                        color: Material.color(Material.Grey)
                        text: qsTr("follows")
                    }
                    Label {
                        Layout.leftMargin: 5
                        font.pointSize: AdjustedValues.f8
                        font.bold: true
                        text: userProfile.followersCount
                    }
                    Label {
                        font.pointSize: AdjustedValues.f8
                        color: Material.color(Material.Grey)
                        text: qsTr("followers")
                    }
                    Label {
                        Layout.leftMargin: 5
                        font.pointSize: AdjustedValues.f8
                        font.bold: true
                        text: userProfile.postsCount
                    }
                    Label {
                        font.pointSize: AdjustedValues.f8
                        color: Material.color(Material.Grey)
                        text: qsTr("posts")
                    }
                }
            }
        }
        RowLayout {
            id: serviceEndpointLayout
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            visible: userProfile.serviceEndpoint.length > 0
            spacing: 1
            Image {
                Layout.preferredWidth: AdjustedValues.i12
                Layout.preferredHeight: AdjustedValues.i12
                source: "../images/database.png"
                layer.enabled: true
                layer.effect: ColorOverlayC {
                    color: Material.color(Material.Grey)
                }
            }
            Label {
                Layout.topMargin: 2
                font.pointSize: AdjustedValues.f8
                color: Material.color(Material.Grey)
                text: userProfile.serviceEndpoint
            }
        }
        RowLayout {
            id: registrationDateLayout
            Layout.topMargin: 2
            Layout.leftMargin: 6
            Layout.rightMargin: 5
            Layout.bottomMargin: 0
            visible: userProfile.registrationDate.length > 0
            spacing: 1
            Label {
                font.pointSize: AdjustedValues.f8
                color: Material.color(Material.Grey)
                text: qsTr("Regisitration date:")
            }
            Label {
                Layout.topMargin: 2
                font.pointSize: AdjustedValues.f8
                color: Material.color(Material.Grey)
                text: userProfile.registrationDate
            }
        }
        TagLabelLayout {
            id: belongingListsLayout
            Layout.preferredWidth: profileView.width - 10
            Layout.maximumWidth: profileView.width
            Layout.topMargin: 5
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            visible: count > 0
            iconSource: "../images/list.png"
            tagSpacing: 5
            tagColor: Material.background
            tagBorderWidth: 1
            model: userProfile.belongingLists
        }
        TagLabelLayout {
            Layout.preferredWidth: profileView.width - 10
            Layout.maximumWidth: profileView.width
            Layout.topMargin: 5
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            visible: count > 0
            model: userProfile.labels
            iconSources: userProfile.labelIcons
        }
        Label {
            id: descriptionLabel
            Layout.topMargin: 5
            Layout.preferredWidth: profileView.width
            Layout.preferredHeight: 0
            wrapMode: Text.Wrap
            lineHeight: 1.1
            font.pointSize: AdjustedValues.f10
            textFormat: Text.StyledText
            text: userProfile.formattedDescription
            leftPadding: 5
            rightPadding: 5

            onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
            onLinkActivated: (url) => Qt.openUrlExternally(url)

            onContentHeightChanged: Layout.preferredHeight = descriptionLabel.contentHeight
            Behavior on Layout.preferredHeight {
                NumberAnimation { duration: 500 }
            }

            IconButton {
                id: moreButton
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: AdjustedValues.b24
                iconSource: "../images/more.png"
                iconSize: AdjustedValues.i16
                foreground: Material.color(Material.Grey)
                onClicked: morePopup.open()
                MenuEx {
                    id: morePopup
                    Action {
                        text: qsTr("Send mention")
                        icon.source: "../images/reply.png"
                        enabled: userProfile.handle.length > 0
                        onTriggered: requestMention("@" + userProfile.handle)
                    }
                    Action {
                        text: qsTr("Send message")
                        icon.source: "../images/chat.png"
                        enabled: userProfile.handle.length > 0 && userProfile.associatedChatAllow
                        onTriggered: requestMessage(userProfile.did)
                    }
                    MenuSeparator {}
                    Action {
                        text: qsTr("Copy handle")
                        icon.source: "../images/copy.png"
                        enabled: userProfile.handle.length > 0
                        onTriggered: systemTool.copyToClipboard("@" + userProfile.handle)
                    }
                    Action {
                        text: qsTr("Copy DID")
                        icon.source: "../images/copy.png"
                        enabled: userProfile.did.length > 0
                        onTriggered: systemTool.copyToClipboard(userProfile.did)
                    }
                    Action {
                        text: qsTr("Copy Official Url")
                        icon.source: "../images/copy.png"
                        enabled: userProfile.did.length > 0
                        onTriggered: systemTool.copyToClipboard(getOfficialUrl(userProfile.did))
                    }
                    MenuSeparator {}
                    Action {
                        text: qsTr("Open in new col")
                        icon.source: "../images/add.png"
                        enabled: userProfile.handle.length > 0
                        onTriggered: requestViewAuthorFeed(userProfile.did, "@" + userProfile.handle)
                    }
                    Action {
                        text: qsTr("Open in Official")
                        icon.source: "../images/open_in_other.png"
                        enabled: userProfile.did.length > 0
                        onTriggered: openInOhters(userProfile.did)
                    }
                    MenuSeparator {}
                    Action {
                        text: qsTr("Add/Remove from lists")
                        icon.source: "../images/list.png"
                        enabled: userProfile.handle.length > 0
                        onTriggered: requestAddRemoveFromLists(userProfile.did)
                    }
                    MenuSeparator {}
                    Action {
                        text: userProfile.muted ? qsTr("Unmute account") : qsTr("Mute account")
                        icon.source: userProfile.muted ? "../images/visibility_on.png" : "../images/visibility_off.png"
                        enabled: userProfile.handle.length > 0 && profileView.userDid !== profileView.accountDid
                        onTriggered: {
                            if(userProfile.muted){
                                recordOperator.deleteMute(userProfile.did)
                            }else{
                                recordOperator.mute(userProfile.did)
                            }
                        }
                    }
                    Action {
                        text: userProfile.blocking ? qsTr("Unblock account") : qsTr("Block account")
                        icon.source: userProfile.blocking ? "../images/block.png" : "../images/block.png"
                        enabled: userProfile.handle.length > 0 && profileView.userDid !== profileView.accountDid
                        onTriggered: {
                            if(userProfile.blocking){
                                recordOperator.deleteBlock(userProfile.blockingUri)
                            }else{
                                recordOperator.block(userProfile.did)
                            }
                        }
                    }
                    Action {
                        text: qsTr("Report account")
                        icon.source: "../images/report.png"
                        enabled: userProfile.handle.length > 0 && profileView.userDid !== profileView.accountDid
                        onTriggered: requestReportAccount(userProfile.did)
                    }
                }
            }
        }

        KnownFollowersInfo {
            id: knownFollowersInfo
            Layout.preferredWidth: profileView.width
            visible: userProfile.knownFollowerAvators.length > 0 && userDid !== accountDid
            model: userProfile.knownFollowerAvators
            knownName: userProfile.knownFollowers.length > 0 ? userProfile.knownFollowers[0] : ""
            othersCount: userProfile.knownFollowersCount - 1
            onClicked: profileView.requestViewKnownFollowers(profileView.userDid)
        }

        IconLabelFrame {
            id: moderationFrame
            Layout.preferredWidth: profileView.width
            visible: false
            states: [
                State {
                    when: userProfile.blocking
                    PropertyChanges {
                        target: moderationFrame
                        visible: true
                        backgroundColor: Material.color(Material.Red)
                        borderWidth: 0
                        iconSource: "../images/block.png"
                        labelText: qsTr("Account blocked")
                    }
                },
                State {
                    when: userProfile.muted
                    PropertyChanges {
                        target: moderationFrame
                        visible: true
                        backgroundColor: Material.color(Material.Grey)
                        borderWidth: 0
                        iconSource: "../images/visibility_off.png"
                        labelText: qsTr("Account muted")
                    }
                }
            ]
        }
        IconLabelFrame {
            id: moderationFrame2
            Layout.topMargin: 2
            Layout.preferredWidth: profileView.width
            visible: userProfile.blockedBy
            backgroundColor: Material.color(Material.Red)
            borderWidth: 0
            iconSource: "../images/block.png"
            labelText: qsTr("This account has blocked you")
        }
    }

    ProfileTabBar {
        id: tabBar
        Layout.fillWidth: true
        Layout.topMargin: 3
        profileSource: userProfile.avatar
    }

    SwipeView {
        id: swipView
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: tabBar.currentIndex
        interactive: false

        onCurrentItemChanged: getLatest()

        function getLatest() {
            if(currentItem == null)
                return
            if(currentItem.model === undefined)
                return
            if(currentItem.model.rowCount() > 0)
                return
            if(userProfile.handle.length === 0)
                return
            currentItem.model.getLatest()
        }

        TimelineView {
            Layout.fillWidth: true
            model: AuthorFeedListModel {
                id: authorFeedListModel
                autoLoading: false
                displayInterval: 0
                authorDid: profileView.userDid
                filter: AuthorFeedListModel.PostsWithReplies
                pinnedPost: userProfile.pinnedPost
                displayPinnedPost: true

                onUpdatePin: (uri) => {
                                 if(userDid === accountDid){
                                     userProfile.pinnedPost = uri
                                     getLatest()
                                 }
                             }
                onErrorOccured: (code, message) => profileView.errorOccured(code, message)
            }
            blogModel: BlogEntryListModel {
                id: authorBlogEntryListModel
                targetHandle: userProfile.handle
                targetDid: profileView.userDid
                targetServiceEndpoint: userProfile.serviceEndpoint
                onTargetServiceEndpointChanged: {
                    if(targetServiceEndpoint.length > 0){
                        getLatest()
                    }
                }
            }

            accountDid: profileView.accountDid
            imageLayoutType: profileView.imageLayoutType

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestReply(cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestQuote(cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => profileView.requestViewThread(uri)
            onRequestViewQuotes: (uri) => profileView.requestViewQuotes(uri)
            onRequestViewImages: (index, paths, alts) => profileView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => {
                                      if(did !== profileView.userDid){
                                          profileView.requestViewProfile(did)
                                      }
                                  }
            onRequestViewFeedGenerator: (name, uri) => profileView.requestViewFeedGenerator(name, uri)
            onRequestViewListFeed: (uri, name) => profileView.requestViewListFeed(uri, name)
            onRequestViewLikedBy: (uri) => profileView.requestViewLikedBy(uri)
            onRequestViewRepostedBy: (uri) => profileView.requestViewRepostedBy(uri)
            onRequestViewSearchPosts: (text) => profileView.requestViewSearchPosts(text)
            onRequestReportPost: (uri, cid) => profileView.requestReportPost(uri, cid)
            onRequestAddMutedWord: (text) => profileView.requestAddMutedWord(text)
            onRequestUpdateThreadGate: (uri, threadgate_uri, type, rules, callback) => profileView.requestUpdateThreadGate(uri, threadgate_uri, type, rules, callback)
            onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
            onScrollPositionChanged: (top) => userProfileColumnLayout.viewChange(top)
        }

        TimelineView {
            Layout.fillWidth: true
            model: AnyFeedListModel {
                id: repostFeedListModel
                autoLoading: false
                displayInterval: 0
                targetDid: profileView.userDid
                targetServiceEndpoint: userProfile.serviceEndpoint
                feedType: AnyFeedListModel.RepostFeedType

                onErrorOccured: (code, message) => profileView.errorOccured(code, message)
            }
            accountDid: profileView.accountDid
            imageLayoutType: profileView.imageLayoutType

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestReply(cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestQuote(cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => profileView.requestViewThread(uri)
            onRequestViewQuotes: (uri) => profileView.requestViewQuotes(uri)
            onRequestViewImages: (index, paths, alts) => profileView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => {
                                      if(did !== profileView.userDid){
                                          profileView.requestViewProfile(did)
                                      }
                                  }
            onRequestViewFeedGenerator: (name, uri) => profileView.requestViewFeedGenerator(name, uri)
            onRequestViewListFeed: (uri, name) => profileView.requestViewListFeed(uri, name)
            onRequestViewLikedBy: (uri) => profileView.requestViewLikedBy(uri)
            onRequestViewRepostedBy: (uri) => profileView.requestViewRepostedBy(uri)
            onRequestViewSearchPosts: (text) => profileView.requestViewSearchPosts(text)
            onRequestReportPost: (uri, cid) => profileView.requestReportPost(uri, cid)
            onRequestAddMutedWord: (text) => profileView.requestAddMutedWord(text)
            onRequestUpdateThreadGate: (uri, threadgate_uri, type, rules, callback) => profileView.requestUpdateThreadGate(uri, threadgate_uri, type, rules, callback)
            onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
            onScrollPositionChanged: (top) => userProfileColumnLayout.viewChange(top)
        }

        TimelineView {
            Layout.fillWidth: true
            model: AnyFeedListModel {
                id: likesFeedListModel
                autoLoading: false
                displayInterval: 0
                targetDid: profileView.userDid
                targetServiceEndpoint: userProfile.serviceEndpoint
                feedType: AnyFeedListModel.LikeFeedType

                onErrorOccured: (code, message) => profileView.errorOccured(code, message)
            }
            accountDid: profileView.accountDid
            imageLayoutType: profileView.imageLayoutType

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestReply(cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestQuote(cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => profileView.requestViewThread(uri)
            onRequestViewQuotes: (uri) => profileView.requestViewQuotes(uri)
            onRequestViewImages: (index, paths, alts) => profileView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => {
                                      if(did !== profileView.userDid){
                                          profileView.requestViewProfile(did)
                                      }
                                  }
            onRequestViewFeedGenerator: (name, uri) => profileView.requestViewFeedGenerator(name, uri)
            onRequestViewListFeed: (uri, name) => profileView.requestViewListFeed(uri, name)
            onRequestViewLikedBy: (uri) => profileView.requestViewLikedBy(uri)
            onRequestViewRepostedBy: (uri) => profileView.requestViewRepostedBy(uri)
            onRequestViewSearchPosts: (text) => profileView.requestViewSearchPosts(text)
            onRequestReportPost: (uri, cid) => profileView.requestReportPost(uri, cid)
            onRequestAddMutedWord: (text) => profileView.requestAddMutedWord(text)
            onRequestUpdateThreadGate: (uri, threadgate_uri, type, rules, callback) => profileView.requestUpdateThreadGate(uri, threadgate_uri, type, rules, callback)
            onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
            onScrollPositionChanged: (top) => userProfileColumnLayout.viewChange(top)
        }

        TimelineView {
            Layout.fillWidth: true
            model: AuthorFeedListModel {
                id: authorMediaFeedListModel
                autoLoading: false
                displayInterval: 0
                authorDid: profileView.userDid
                filter: AuthorFeedListModel.PostsWithMedia

                onErrorOccured: (code, message) => profileView.errorOccured(code, message)
            }
            accountDid: profileView.accountDid
            imageLayoutType: profileView.imageLayoutType

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestReply(cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestQuote(cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => profileView.requestViewThread(uri)
            onRequestViewQuotes: (uri) => profileView.requestViewQuotes(uri)
            onRequestViewImages: (index, paths, alts) => profileView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => {
                                      if(did !== profileView.userDid){
                                          profileView.requestViewProfile(did)
                                      }
                                  }
            onRequestViewFeedGenerator: (name, uri) => profileView.requestViewFeedGenerator(name, uri)
            onRequestViewListFeed: (uri, name) => profileView.requestViewListFeed(uri, name)
            onRequestViewLikedBy: (uri) => profileView.requestViewLikedBy(uri)
            onRequestViewRepostedBy: (uri) => profileView.requestViewRepostedBy(uri)
            onRequestViewSearchPosts: (text) => profileView.requestViewSearchPosts(text)
            onRequestReportPost: (uri, cid) => profileView.requestReportPost(uri, cid)
            onRequestAddMutedWord: (text) => profileView.requestAddMutedWord(text)
            onRequestUpdateThreadGate: (uri, threadgate_uri, type, rules, callback) => profileView.requestUpdateThreadGate(uri, threadgate_uri, type, rules, callback)
            onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
            onScrollPositionChanged: (top) => userProfileColumnLayout.viewChange(top)
        }

        TimelineView {
            Layout.fillWidth: true
            model: AuthorFeedListModel {
                id: authorVideoFeedListModel
                autoLoading: false
                displayInterval: 0
                authorDid: profileView.userDid
                filter: AuthorFeedListModel.PostsWithVideo

                onErrorOccured: (code, message) => profileView.errorOccured(code, message)
            }
            accountDid: profileView.accountDid
            imageLayoutType: profileView.imageLayoutType

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestReply(cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestQuote(cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => profileView.requestViewThread(uri)
            onRequestViewQuotes: (uri) => profileView.requestViewQuotes(uri)
            onRequestViewImages: (index, paths, alts) => profileView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => {
                                      if(did !== profileView.userDid){
                                          profileView.requestViewProfile(did)
                                      }
                                  }
            onRequestViewFeedGenerator: (name, uri) => profileView.requestViewFeedGenerator(name, uri)
            onRequestViewListFeed: (uri, name) => profileView.requestViewListFeed(uri, name)
            onRequestViewLikedBy: (uri) => profileView.requestViewLikedBy(uri)
            onRequestViewRepostedBy: (uri) => profileView.requestViewRepostedBy(uri)
            onRequestViewSearchPosts: (text) => profileView.requestViewSearchPosts(text)
            onRequestReportPost: (uri, cid) => profileView.requestReportPost(uri, cid)
            onRequestAddMutedWord: (text) => profileView.requestAddMutedWord(text)
            onRequestUpdateThreadGate: (uri, threadgate_uri, type, rules, callback) => profileView.requestUpdateThreadGate(uri, threadgate_uri, type, rules, callback)
            onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
            onScrollPositionChanged: (top) => userProfileColumnLayout.viewChange(top)
        }

        FeedGeneratorListView {
            id: generatorScrollView
            Layout.fillWidth: true
            selectable: false
            model: ActorFeedGeneratorListModel {
                id: actorFeedGeneratorListModel
                displayInterval: 0
                actor: profileView.userDid
            }
            onClicked: (display_name, uri) => profileView.requestViewFeedGenerator(display_name, uri)
            onRequestRemoveGenerator: (uri) => actorFeedGeneratorListModel.removeGenerator(uri)
            onRequestSaveGenerator: (uri) => actorFeedGeneratorListModel.saveGenerator(uri)
            onScrollPositionChanged: (top) => userProfileColumnLayout.viewChange(top)
        }

        ListsListView {
            id: listsListView
            Layout.fillWidth: true
            model: ListsListModel {
                id: listsListModel
                displayInterval: 0
                actor: profileView.userDid
                visibilityType: ListsListModel.VisibilityTypeAll
            }
            onRequestViewListDetail: (uri) => profileView.requestViewListDetail(uri)
            onScrollPositionChanged: (top) => userProfileColumnLayout.viewChange(top)
        }

        ProfileListView {
            id: followsListView
            Layout.fillWidth: true
            userDid: profileView.userDid
            accountDid: profileView.accountDid
            model: FollowsListModel {
                id: followsListModel
                autoLoading: false
                displayInterval: 0
                targetDid: profileView.userDid

                onErrorOccured: (code, message) => profileView.errorOccured(code, message)
            }
            onRequestViewProfile: (did) => {
                                      if(did !== profileView.userDid){
                                          profileView.requestViewProfile(did)
                                      }
                                  }
            onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
            onScrollPositionChanged: (top) => userProfileColumnLayout.viewChange(top)
        }

        ProfileListView {
            id: followersListView
            Layout.fillWidth: true
            userDid: profileView.userDid
            accountDid: profileView.accountDid
            unfollowAndRemove: false
            model: FollowersListModel {
                id: followersListModel
                autoLoading: false
                displayInterval: 0
                targetDid: profileView.userDid

                onErrorOccured: (code, message) => profileView.errorOccured(code, message)
            }
            onRequestViewProfile: (did) => {
                                      if(did !== profileView.userDid){
                                          profileView.requestViewProfile(did)
                                      }
                                  }
            onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
            onScrollPositionChanged: (top) => userProfileColumnLayout.viewChange(top)
        }
    }
}
