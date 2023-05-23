import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.userprofile 1.0
import tech.relog.hagoromo.authorfeedlistmodel 1.0
import tech.relog.hagoromo.anyfeedlistmodel 1.0

import "../parts"
import "../controls"

ColumnLayout {
    id: profileView
    Layout.fillWidth: true
    Layout.fillHeight: true
    spacing: 0

    property alias model: relayObject


    property string userDid: ""

    signal requestedReply(string cid, string uri,
                          string reply_root_cid, string reply_root_uri,
                          string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestedRepost(string cid, string uri)
    signal requestedQuote(string cid, string uri, string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestedLike(string cid, string uri)
    signal requestedViewThread(string uri)
    signal requestedViewImages(int index, string paths)
    signal requestedViewProfile(string did)

    signal back()

    QtObject {
        id: relayObject
        function rowCount() {
            return userProfile.handle.length;
        }
        function setAccount(service, did, handle, email, accessJwt, refreshJwt) {
            userProfile.setAccount(service, did, handle, email, accessJwt, refreshJwt)
            authorFeedListModel.setAccount(service, did, handle, email, accessJwt, refreshJwt)
            repostFeedListModel.setAccount(service, did, handle, email, accessJwt, refreshJwt)
            likesFeedListModel.setAccount(service, did, handle, email, accessJwt, refreshJwt)
        }
        function getLatest() {
            userProfile.getProfile(userDid)
            authorFeedListModel.getLatest()
            repostFeedListModel.getLatest()
            likesFeedListModel.getLatest()
        }
    }

    UserProfile {
        id: userProfile
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
                text: "Profile"
            }
        }
    }


    ColumnLayout {
        Layout.fillWidth: true
        Layout.margins: 0
        ImageWithIndicator {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            fillMode: Image.PreserveAspectCrop
            source: userProfile.banner
        }
        GridLayout {
            columns: 2
            AvatarImage {
                id: avatarImage
                Layout.preferredWidth: 48
                Layout.preferredHeight: 48
                Layout.rowSpan: 2
                source: userProfile.avatar
            }
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
        }
        Label {
            Layout.preferredWidth: profileView.width
            wrapMode: Text.Wrap
            text: userProfile.description
        }
    }

    TabBar {
        id: tabBar
        Layout.fillWidth: true

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
    }

    SwipeView {
        id: swipView
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: tabBar.currentIndex
        interactive: false

        TimelineView {
            Layout.fillWidth: true
            model: AuthorFeedListModel {
                id: authorFeedListModel
                autoLoading: false
                authorDid: profileView.userDid
            }

            onRequestedReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                              profileView.requestedReply(profileView.accountUuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestedRepost: (cid, uri) => profileView.repost(cid, uri)
            onRequestedQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                              profileView.requestedQuote(profileView.accountUuid, cid, uri, avatar, display_name, handle, indexed_at, text)
            onRequestedLike: (cid, uri) => profileView.like(cid, uri)

            onRequestedViewThread: (uri) => profileView.requestedViewThread(uri)
            onRequestedViewImages: (index, paths) => profileView.requestedViewImages(index, paths)
            onRequestedViewProfile: (did) => profileView.requestedViewProfile(did)
        }

        TimelineView {
            Layout.fillWidth: true
            model: AnyFeedListModel {
                id: repostFeedListModel
                autoLoading: false
                targetDid: profileView.userDid
                feedType: AnyFeedListModel.RepostFeedType
            }

            onRequestedReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                              profileView.requestedReply(profileView.accountUuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestedRepost: (cid, uri) => profileView.repost(cid, uri)
            onRequestedQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                              profileView.requestedQuote(profileView.accountUuid, cid, uri, avatar, display_name, handle, indexed_at, text)
            onRequestedLike: (cid, uri) => profileView.like(cid, uri)

            onRequestedViewThread: (uri) => profileView.requestedViewThread(uri)
            onRequestedViewImages: (index, paths) => profileView.requestedViewImages(index, paths)
            onRequestedViewProfile: (did) => profileView.requestedViewProfile(did)
        }

        TimelineView {
            Layout.fillWidth: true
            model: AnyFeedListModel {
                id: likesFeedListModel
                autoLoading: false
                targetDid: profileView.userDid
                feedType: AnyFeedListModel.LikeFeedType
            }

            onRequestedReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                              profileView.requestedReply(profileView.accountUuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestedRepost: (cid, uri) => profileView.repost(cid, uri)
            onRequestedQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                              profileView.requestedQuote(profileView.accountUuid, cid, uri, avatar, display_name, handle, indexed_at, text)
            onRequestedLike: (cid, uri) => profileView.like(cid, uri)

            onRequestedViewThread: (uri) => profileView.requestedViewThread(uri)
            onRequestedViewImages: (index, paths) => profileView.requestedViewImages(index, paths)
            onRequestedViewProfile: (did) => profileView.requestedViewProfile(did)
        }
    }
}
