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
import tech.relog.hagoromo.systemtool 1.0

import "../parts"
import "../controls"

ColumnLayout {
    id: profileView
    Layout.fillWidth: true
    Layout.fillHeight: true
    spacing: 0

    property string hoveredLink: ""
    property real fontSizeRatio: 1.0

    property alias model: relayObject

    property string userDid: ""     // 表示するアカウント
    property string accountDid: ""  // 認証しているアカウント

    signal requestReply(string cid, string uri,
                          string reply_root_cid, string reply_root_uri,
                          string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestQuote(string cid, string uri, string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestMention(string handle)
    signal requestViewThread(string uri)
    signal requestViewImages(int index, string paths)
    signal requestViewProfile(string did)

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
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                flat: true
                iconSource: "../images/arrow_left_single.png"
                onClicked: profileView.back()
            }
            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 10
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
                    font.pointSize: 8
                    color: Material.accentColor
                    text: qsTr("Follows you")
                }
                IconButton {
                    id: editButton
                    Layout.preferredHeight: 24
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
                Layout.preferredWidth: 48
                Layout.preferredHeight: 48
                Layout.rowSpan: 2
                source: userProfile.avatar
            }
            ColumnLayout {
                Label {
                    Layout.preferredWidth: profileView.width - avatarImage.width - parent.columnSpacing
                    font.pointSize: 12
                    elide: Text.ElideRight
                    text: userProfile.displayName
                }
                Label {
                    Layout.preferredWidth: profileView.width - avatarImage.width - parent.columnSpacing
                    elide: Text.ElideRight
                    font.pointSize: 8
                    color: Material.color(Material.Grey)
                    text: "@" + userProfile.handle
                }
                RowLayout {
                    spacing: 3
                    Label {
                        font.pointSize: 8
                        font.bold: true
                        text: userProfile.followsCount
                    }
                    Label {
                        font.pointSize: 8
                        color: Material.color(Material.Grey)
                        text: qsTr("follows")
                    }
                    Label {
                        Layout.leftMargin: 5
                        font.pointSize: 8
                        font.bold: true
                        text: userProfile.followersCount
                    }
                    Label {
                        font.pointSize: 8
                        color: Material.color(Material.Grey)
                        text: qsTr("followers")
                    }
                    Label {
                        Layout.leftMargin: 5
                        font.pointSize: 8
                        font.bold: true
                        text: userProfile.postsCount
                    }
                    Label {
                        font.pointSize: 8
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
            font.pointSize: 10
            textFormat: Text.StyledText
            text: userProfile.description

            onHoveredLinkChanged: profileView.hoveredLink = hoveredLink
            onLinkActivated: (url) => Qt.openUrlExternally(url)

            IconButton {
                id: moreButton
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 24
                iconSource: "../images/more.png"
                iconSize: 16
                foreground: Material.color(Material.Grey)
                flat: true
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
                        text: qsTr("Open in Official")
                        icon.source: "../images/open_in_other.png"
                        enabled: userProfile.handle.length > 0
                        onTriggered: openInOhters(userProfile.handle)
                    }
                }
            }
        }
    }

    TabBar {
        id: tabBar
        Layout.fillWidth: true
        Layout.topMargin: 3

        TabButton {
            font.capitalization: Font.MixedCase
            text: qsTr("Posts")
        }
        TabButton {
            font.capitalization: Font.MixedCase
            text: qsTr("Reposts")
        }
        TabButton {
            font.capitalization: Font.MixedCase
            text: qsTr("Likes")
        }
        TabButton {
            font.capitalization: Font.MixedCase
            text: qsTr("Follows")
        }
        TabButton {
            font.capitalization: Font.MixedCase
            text: qsTr("Followers")
        }
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

                onErrorOccured: (message) => {console.log(message)}
            }
            fontSizeRatio: profileView.fontSizeRatio
            accountDid: profileView.accountDid

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                              profileView.requestReply(cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                              profileView.requestQuote(cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => profileView.requestViewThread(uri)
            onRequestViewImages: (index, paths) => profileView.requestViewImages(index, paths)
            onRequestViewProfile: (did) => {
                                        if(did !== profileView.userDid){
                                            profileView.requestViewProfile(did)
                                        }
                                    }
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
            fontSizeRatio: profileView.fontSizeRatio
            accountDid: profileView.accountDid

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                              profileView.requestReply(cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                              profileView.requestQuote(cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => profileView.requestViewThread(uri)
            onRequestViewImages: (index, paths) => profileView.requestViewImages(index, paths)
            onRequestViewProfile: (did) => {
                                        if(did !== profileView.userDid){
                                            profileView.requestViewProfile(did)
                                        }
                                    }
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
            fontSizeRatio: profileView.fontSizeRatio
            accountDid: profileView.accountDid

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                              profileView.requestReply(cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                              profileView.requestQuote(cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => profileView.requestViewThread(uri)
            onRequestViewImages: (index, paths) => profileView.requestViewImages(index, paths)
            onRequestViewProfile: (did) => {
                                        if(did !== profileView.userDid){
                                            profileView.requestViewProfile(did)
                                        }
                                    }
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
