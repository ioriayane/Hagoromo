import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.singleton 1.0

import "../controls"

RowLayout {
    spacing: 10

    property bool isReposted: false
    property bool isLiked: false
    property bool pinned: false
    property string postUri: ""
    property string handle: ""
    property bool mine: false

    property alias replyButton: replyButton
    property alias repostButton: repostButton
    property alias repostMenuItem: repostMenuItem
    property alias quoteMenuItem: quoteMenuItem
    property alias likeButton: likeButton
    property alias pinnedImage: pinnedImage

    signal triggeredTranslate()
    signal triggeredCopyToClipboard()
    signal triggeredCopyPostUrlToClipboard()
    signal triggeredDeletePost()
    signal triggeredRequestReport()
    signal triggeredRequestViewLikedBy()
    signal triggeredRequestViewRepostedBy()
    signal triggeredRequestUpdateThreadGate()
    signal triggeredRequestPin()


    function openInOhters(uri, handle){
        if(uri.length === 0 || uri.startsWith("at://") === false){
            return
        }
        var items = uri.split("/")
        var url = "https://bsky.app/profile/" + handle + "/post/" + items[items.length-1]

        Qt.openUrlExternally(url)
    }

    IconButton {
        id: replyButton
        Layout.preferredHeight: AdjustedValues.b24
        iconSource: "../images/reply.png"
        iconSize: AdjustedValues.i16
        foreground: Material.color(Material.Grey)
        flat: true
    }
    IconButton {
        id: repostButton
        Layout.preferredHeight: AdjustedValues.b24
        iconSource: "../images/repost.png"
        iconSize: AdjustedValues.i16
        foreground: isReposted ? Material.color(Material.Green) : Material.color(Material.Grey)
        flat: true
        onClicked: popup.open()
        Menu {
            id: popup
            MenuItem {
                id: repostMenuItem
                icon.source: "../images/repost.png"
                text: qsTr("Repost")
            }
            MenuItem {
                id: quoteMenuItem
                icon.source: "../images/quote.png"
                text: qsTr("Quote")
            }
        }
        BusyIndicator {
            anchors.fill: parent
            visible: !parent.enabled
        }
    }
    IconButton {
        id: likeButton
        Layout.preferredHeight: AdjustedValues.b24
        iconSource: "../images/like.png"
        iconSize: AdjustedValues.i16
        foreground: isLiked ? Material.color(Material.Pink) : Material.color(Material.Grey)
        flat: true
        BusyIndicator {
            anchors.fill: parent
            visible: !parent.enabled
        }
    }
    Item {
        id: pinnedImage
        Layout.preferredWidth: AdjustedValues.b24
        Layout.preferredHeight: AdjustedValues.b24
        Layout.leftMargin: 5
        Layout.rightMargin: 5
        Layout.alignment: Qt.AlignVCenter
        Image {
            width: AdjustedValues.i16
            height: AdjustedValues.i16
            anchors.centerIn: parent
            source: pinned ? "../images/pin.png" : ""
            layer.enabled: true
            layer.effect: ColorOverlay {
                color: "white"
            }
        }
        BusyIndicator {
            anchors.fill: parent
            visible: !parent.enabled
        }
    }
    Item {
        Layout.fillWidth: true
        height: 1
    }
    IconButton {
        id: moreButton
        Layout.preferredHeight: AdjustedValues.b24
        iconSource: "../images/more.png"
        iconSize: AdjustedValues.i16
        foreground: Material.color(Material.Grey)
        flat: true
        onClicked:{
            if(mine){
                myMorePopup.open()
            }else{
                theirMorePopup.open()
            }
        }
        Menu {
            id: myMorePopup
            width: 230
            MenuItem {
                icon.source: "../images/translate.png"
                text: qsTr("Translate")
                onTriggered: triggeredTranslate()
            }
            MenuItem {
                icon.source: "../images/copy.png"
                text: qsTr("Copy post text")
                onTriggered: triggeredCopyToClipboard()
            }
            MenuItem {
                text: qsTr("Copy url")
                icon.source: "../images/copy.png"
                onTriggered: triggeredCopyPostUrlToClipboard()
            }
            MenuItem {
                text: qsTr("Open in Official")
                enabled: postUri.length > 0 && handle.length > 0
                icon.source: "../images/open_in_other.png"
                onTriggered: openInOhters(postUri, handle)
            }
            MenuSeparator {}
            MenuItem {
                text: qsTr("Reposted by")
                enabled: repostButton.iconText > 0
                icon.source: "../images/repost.png"
                onTriggered: triggeredRequestViewRepostedBy()
            }
            MenuItem {
                text: qsTr("Liked by")
                enabled: likeButton.iconText > 0
                icon.source: "../images/like.png"
                onTriggered: triggeredRequestViewLikedBy()
            }
            MenuSeparator {}
            MenuItem {
                text: pinned ? qsTr("Unpin this post") : qsTr("Pin this post")
                icon.source: "../images/pin.png"
                onTriggered: triggeredRequestPin()
            }
            MenuItem {
                text: qsTr("Who can reply")
                enabled: mine
                icon.source: "../images/thread.png"
                onTriggered: triggeredRequestUpdateThreadGate()
            }
            MenuItem {
                text: qsTr("Delete post")
                enabled: mine
                icon.source: "../images/delete.png"
                onTriggered: triggeredDeletePost()
            }
            MenuSeparator {}
            MenuItem {
                text: qsTr("Report post")
                icon.source: "../images/report.png"
                onTriggered: triggeredRequestReport()
            }
        }
        Menu {
            id: theirMorePopup
            width: 230
            MenuItem {
                icon.source: "../images/translate.png"
                text: qsTr("Translate")
                onTriggered: triggeredTranslate()
            }
            MenuItem {
                icon.source: "../images/copy.png"
                text: qsTr("Copy post text")
                onTriggered: triggeredCopyToClipboard()
            }
            MenuItem {
                text: qsTr("Copy url")
                icon.source: "../images/copy.png"
                onTriggered: triggeredCopyPostUrlToClipboard()
            }
            MenuItem {
                text: qsTr("Open in Official")
                enabled: postUri.length > 0 && handle.length > 0
                icon.source: "../images/open_in_other.png"
                onTriggered: openInOhters(postUri, handle)
            }
            MenuSeparator {}
            MenuItem {
                text: qsTr("Reposted by")
                enabled: repostButton.iconText > 0
                icon.source: "../images/repost.png"
                onTriggered: triggeredRequestViewRepostedBy()
            }
            MenuItem {
                text: qsTr("Liked by")
                enabled: likeButton.iconText > 0
                icon.source: "../images/like.png"
                onTriggered: triggeredRequestViewLikedBy()
            }
            MenuSeparator {}
            MenuItem {
                text: pinned ? qsTr("Unpin this post") : qsTr("Pin this post")
                icon.source: "../images/pin.png"
                onTriggered: triggeredRequestPin()
            }
            MenuSeparator {}
            MenuItem {
                text: qsTr("Report post")
                icon.source: "../images/report.png"
                onTriggered: triggeredRequestReport()
            }
        }
    }
}
