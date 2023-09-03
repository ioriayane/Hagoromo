import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.userprofile 1.0
import tech.relog.hagoromo.authorfeedlistmodel 1.0
import tech.relog.hagoromo.anyfeedlistmodel 1.0
import tech.relog.hagoromo.recordoperator 1.0
import tech.relog.hagoromo.followslistmodel 1.0
import tech.relog.hagoromo.followerslistmodel 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"

ColumnLayout {
    id: profileView
    Layout.fillWidth: true
    Layout.fillHeight: true
    spacing: 0

    property string hoveredLink: ""

    property alias model: relayObject

    property string userDid: ""     // 表示するアカウント
    property string accountDid: ""  // 認証しているアカウント

    signal requestReply(string cid, string uri,
                        string reply_root_cid, string reply_root_uri,
                        string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestQuote(string cid, string uri, string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestMention(string handle)
    signal requestViewThread(string uri)
    signal requestViewImages(int index, var paths, var alts)
    signal requestViewProfile(string did)
    signal requestViewGeneratorFeed(string name, string uri)
    signal requestViewAuthorFeed(string did, string handle)
    signal requestReportPost(string uri, string cid)
    signal requestReportAccount(string did)

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
            PropertyChanges { target: editButton; visible: false }  // 仮
            PropertyChanges { target: editButton; iconText: qsTr("Edit Profile") }
            PropertyChanges { target: editButton; onClicked: {
                    // edit profile
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

    function openInOhters(handle) {
        if(handle.length === 0){
            return
        }
        Qt.openUrlExternally("https://bsky.app/profile/" + handle)
    }

    QtObject {
        id: relayObject
        function rowCount() {
            return userProfile.handle.length;
        }
        function setAccount(service, did, handle, email, accessJwt, refreshJwt) {
            accountDid = did
            recordOperator.setAccount(service, did, handle, email, accessJwt, refreshJwt)
            userProfile.setAccount(service, did, handle, email, accessJwt, refreshJwt)
            authorFeedListModel.setAccount(service, did, handle, email, accessJwt, refreshJwt)
            repostFeedListModel.setAccount(service, did, handle, email, accessJwt, refreshJwt)
            likesFeedListModel.setAccount(service, did, handle, email, accessJwt, refreshJwt)
            authorMediaFeedListModel.setAccount(service, did, handle, email, accessJwt, refreshJwt)
            followsListModel.setAccount(service, did, handle, email, accessJwt, refreshJwt)
            followersListModel.setAccount(service, did, handle, email, accessJwt, refreshJwt)
        }
        function getLatest() {
            userProfile.getProfile(userDid)
            authorFeedListModel.getLatest()
        }
    }

    RecordOperator {
        id: recordOperator
        onFinished: (success) => userProfile.getProfile(userDid)
    }

    UserProfile {
        id: userProfile
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
        Layout.fillWidth: true
        Layout.topMargin: 0
        Layout.leftMargin: 0
        Layout.rightMargin: 0
        Layout.bottomMargin: 5

        ImageWithIndicator {
            id: bannerImage
            Layout.preferredWidth: profileView.width
            Layout.preferredHeight: 80
            fillMode: Image.PreserveAspectCrop
            source: userProfile.banner

            RowLayout {
                anchors.top: bannerImage.bottom
                anchors.right: bannerImage.right

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
        }
        RowLayout {
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            AvatarImage {
                id: avatarImage
                Layout.preferredWidth: AdjustedValues.i48
                Layout.preferredHeight: AdjustedValues.i48
                Layout.rowSpan: 2
                source: userProfile.avatar
            }
            ColumnLayout {
                Label {
                    Layout.preferredWidth: profileView.width - avatarImage.width - parent.columnSpacing
                    font.pointSize: AdjustedValues.f12
                    elide: Text.ElideRight
                    text: userProfile.displayName
                }
                Label {
                    Layout.preferredWidth: profileView.width - avatarImage.width - parent.columnSpacing
                    elide: Text.ElideRight
                    font.pointSize: AdjustedValues.f8
                    color: Material.color(Material.Grey)
                    text: "@" + userProfile.handle
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
        Label {
            Layout.preferredWidth: profileView.width
            wrapMode: Text.Wrap
            lineHeight: 1.1
            font.pointSize: AdjustedValues.f10
            textFormat: Text.StyledText
            text: userProfile.description

            onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
            onLinkActivated: (url) => Qt.openUrlExternally(url)

            IconButton {
                id: moreButton
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: AdjustedValues.b24
                iconSource: "../images/more.png"
                iconSize: AdjustedValues.i16
                foreground: Material.color(Material.Grey)
                onClicked: morePopup.open()
                Menu {
                    id: morePopup
                    MenuItem {
                        text: qsTr("Send mention")
                        icon.source: "../images/reply.png"
                        enabled: userProfile.handle.length > 0
                        onTriggered: requestMention("@" + userProfile.handle)
                    }
                    MenuSeparator {}
                    MenuItem {
                        text: qsTr("Copy handle")
                        icon.source: "../images/copy.png"
                        enabled: userProfile.handle.length > 0
                        onTriggered: systemTool.copyToClipboard("@" + userProfile.handle)
                    }
                    MenuItem {
                        text: qsTr("Copy DID")
                        icon.source: "../images/copy.png"
                        enabled: userProfile.did.length > 0
                        onTriggered: systemTool.copyToClipboard(userProfile.did)
                    }
                    MenuSeparator {}
                    MenuItem {
                        text: qsTr("Open in new col")
                        icon.source: "../images/add.png"
                        enabled: userProfile.handle.length > 0
                        onTriggered: requestViewAuthorFeed(userProfile.did, "@" + userProfile.handle)
                    }
                    MenuItem {
                        text: qsTr("Open in Official")
                        icon.source: "../images/open_in_other.png"
                        enabled: userProfile.handle.length > 0
                        onTriggered: openInOhters(userProfile.handle)
                    }
                    MenuSeparator {}
                    MenuItem {
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
                    MenuItem {
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
                    MenuItem {
                        text: qsTr("Report account")
                        icon.source: "../images/report.png"
                        enabled: userProfile.handle.length > 0 && profileView.userDid !== profileView.accountDid
                        onTriggered: requestReportAccount(userProfile.did)
                    }
                }
            }
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
            id: moderationFrame3
            Layout.preferredWidth: profileView.width
            visible: userProfile.userFilterMatched
            backgroundColor: Material.color(Material.Red)
            borderWidth: 0
            iconSource: "../images/labeling.png"
            labelText: qsTr("This account has been flagged : ") + userProfile.userFilterTitle
        }
        IconLabelFrame {
            id: moderationFrame2
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

        onCurrentItemChanged: {
            if(currentItem.model === undefined)
                return
            if(currentItem.model.rowCount() > 0)
                return
            currentItem.model.getLatest()
        }

        TimelineView {
            Layout.fillWidth: true
            model: AuthorFeedListModel {
                id: authorFeedListModel
                autoLoading: false
                authorDid: profileView.userDid
                filter: AuthorFeedListModel.PostsWithReplies

                onErrorOccured: (message) => {console.log(message)}
            }
            accountDid: profileView.accountDid

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestReply(cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestQuote(cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => profileView.requestViewThread(uri)
            onRequestViewImages: (index, paths, alts) => profileView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => {
                                      if(did !== profileView.userDid){
                                          profileView.requestViewProfile(did)
                                      }
                                  }
            onRequestViewGeneratorFeed: (name, uri) => profileView.requestViewGeneratorFeed(name, uri)
            onRequestReportPost: (uri, cid) => profileView.requestReportPost(uri, cid)
            onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
        }

        TimelineView {
            Layout.fillWidth: true
            model: AnyFeedListModel {
                id: repostFeedListModel
                autoLoading: false
                targetDid: profileView.userDid
                feedType: AnyFeedListModel.RepostFeedType

                onErrorOccured: (message) => {console.log(message)}
            }
            accountDid: profileView.accountDid

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestReply(cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestQuote(cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => profileView.requestViewThread(uri)
            onRequestViewImages: (index, paths, alts) => profileView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => {
                                      if(did !== profileView.userDid){
                                          profileView.requestViewProfile(did)
                                      }
                                  }
            onRequestViewGeneratorFeed: (name, uri) => profileView.requestViewGeneratorFeed(name, uri)
            onRequestReportPost: (uri, cid) => profileView.requestReportPost(uri, cid)
            onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
        }

        TimelineView {
            Layout.fillWidth: true
            model: AnyFeedListModel {
                id: likesFeedListModel
                autoLoading: false
                targetDid: profileView.userDid
                feedType: AnyFeedListModel.LikeFeedType

                onErrorOccured: (message) => {console.log(message)}
            }
            accountDid: profileView.accountDid

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestReply(cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestQuote(cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => profileView.requestViewThread(uri)
            onRequestViewImages: (index, paths, alts) => profileView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => {
                                      if(did !== profileView.userDid){
                                          profileView.requestViewProfile(did)
                                      }
                                  }
            onRequestViewGeneratorFeed: (name, uri) => profileView.requestViewGeneratorFeed(name, uri)
            onRequestReportPost: (uri, cid) => profileView.requestReportPost(uri, cid)
            onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
        }

        TimelineView {
            Layout.fillWidth: true
            model: AuthorFeedListModel {
                id: authorMediaFeedListModel
                autoLoading: false
                authorDid: profileView.userDid
                filter: AuthorFeedListModel.PostsWithMedia

                onErrorOccured: (message) => {console.log(message)}
            }
            accountDid: profileView.accountDid

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestReply(cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            profileView.requestQuote(cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => profileView.requestViewThread(uri)
            onRequestViewImages: (index, paths, alts) => profileView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => {
                                      if(did !== profileView.userDid){
                                          profileView.requestViewProfile(did)
                                      }
                                  }
            onRequestViewGeneratorFeed: (name, uri) => profileView.requestViewGeneratorFeed(name, uri)
            onRequestReportPost: (uri, cid) => profileView.requestReportPost(uri, cid)
            onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
        }

        ProfileListView {
            id: followsListView
            Layout.fillWidth: true
            userDid: profileView.userDid
            accountDid: profileView.accountDid
            model: FollowsListModel {
                id: followsListModel
                autoLoading: false
                targetDid: profileView.userDid

                onErrorOccured: (message) => {console.log(message)}
            }
            onRequestViewProfile: (did) => {
                                      if(did !== profileView.userDid){
                                          profileView.requestViewProfile(did)
                                      }
                                  }
            onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
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
                targetDid: profileView.userDid

                onErrorOccured: (message) => {console.log(message)}
            }
            onRequestViewProfile: (did) => {
                                      if(did !== profileView.userDid){
                                          profileView.requestViewProfile(did)
                                      }
                                  }
            onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
        }
    }
}
