import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../compat"

RowLayout {
    spacing: 10

    property bool isReposted: false
    property bool isLiked: false
    property bool pinned: false
    property bool threadMuted: false
    property string postUri: ""
    property string handle: ""
    property bool mine: false
    property bool logMode: false
    property int repostCount: 0
    property int quoteCount: 0

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
    signal triggeredRequestViewQuotes()
    signal triggeredRequestUpdateThreadGate()
    signal triggeredRequestPin()
    signal triggeredRequestMuteThread()

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
        iconText: repostCount + (quoteCount > 0 ? "+" + quoteCount : "")
        onClicked: popup.open()
        MenuEx {
            id: popup
            Action {
                id: repostMenuItem
                icon.source: "../images/repost.png"
                text: qsTr("Repost")
            }
            Action {
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
            visible: pinned
            source: "../images/pin.png"
            layer.enabled: true
            layer.effect: ColorOverlayC {
                color: Material.foreground
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
        MenuEx {
            id: myMorePopup
            Action {
                icon.source: "../images/translate.png"
                text: qsTr("Translate")
                onTriggered: triggeredTranslate()
            }
            Action {
                icon.source: "../images/copy.png"
                text: qsTr("Copy post text")
                onTriggered: triggeredCopyToClipboard()
            }
            Action {
                text: qsTr("Copy url")
                icon.source: "../images/copy.png"
                onTriggered: triggeredCopyPostUrlToClipboard()
            }
            Action {
                text: qsTr("Open in Official")
                enabled: postUri.length > 0 && handle.length > 0
                icon.source: "../images/open_in_other.png"
                onTriggered: openInOhters(postUri, handle)
            }
            MenuSeparator {}
            Action {
                text: qsTr("Reposted by")
                enabled: repostCount > 0 && !logMode
                icon.source: "../images/repost.png"
                onTriggered: triggeredRequestViewRepostedBy()
            }
            Action {
                text: qsTr("Liked by")
                enabled: likeButton.iconText > 0 && !logMode
                icon.source: "../images/like.png"
                onTriggered: triggeredRequestViewLikedBy()
            }
            Action {
                text: qsTr("Quotes")
                enabled: quoteCount > 0 && !logMode
                icon.source: "../images/quote.png"
                onTriggered: triggeredRequestViewQuotes()
            }
            MenuSeparator {}
            Action {
                text: pinned ? qsTr("Unpin this post") : qsTr("Pin this post")
                icon.source: "../images/pin.png"
                onTriggered: triggeredRequestPin()
            }
            MenuSeparator {}
            Action {
                text: threadMuted ? qsTr("Unmute thread") : qsTr("Mute thread")
                icon.source: "../images/mute.png"
                onTriggered: triggeredRequestMuteThread()
            }
            Action {
                text: qsTr("Who can reply")
                enabled: mine
                icon.source: "../images/thread.png"
                onTriggered: triggeredRequestUpdateThreadGate()
            }
            MenuSeparator {}
            Action {
                text: qsTr("Delete post")
                enabled: mine
                icon.source: "../images/delete.png"
                onTriggered: triggeredDeletePost()
            }
        }
        MenuEx {
            id: theirMorePopup
            width: 230
            Action {
                icon.source: "../images/translate.png"
                text: qsTr("Translate")
                onTriggered: triggeredTranslate()
            }
            Action {
                icon.source: "../images/copy.png"
                text: qsTr("Copy post text")
                onTriggered: triggeredCopyToClipboard()
            }
            Action {
                text: qsTr("Copy url")
                icon.source: "../images/copy.png"
                onTriggered: triggeredCopyPostUrlToClipboard()
            }
            Action {
                text: qsTr("Open in Official")
                enabled: postUri.length > 0 && handle.length > 0
                icon.source: "../images/open_in_other.png"
                onTriggered: openInOhters(postUri, handle)
            }
            MenuSeparator {}
            Action {
                text: qsTr("Reposted by")
                enabled: repostCount > 0
                icon.source: "../images/repost.png"
                onTriggered: triggeredRequestViewRepostedBy()
            }
            Action {
                text: qsTr("Liked by")
                enabled: likeButton.iconText > 0
                icon.source: "../images/like.png"
                onTriggered: triggeredRequestViewLikedBy()
            }
            Action {
                text: qsTr("Quotes")
                enabled: quoteCount > 0
                icon.source: "../images/quote.png"
                onTriggered: triggeredRequestViewQuotes()
            }
            MenuSeparator {}
            Action {
                text: pinned ? qsTr("Unpin this post") : qsTr("Pin this post")
                icon.source: "../images/pin.png"
                onTriggered: triggeredRequestPin()
            }
            MenuSeparator {}
            Action {
                text: threadMuted ? qsTr("Unmute thread") : qsTr("Mute thread")
                icon.source: "../images/mute.png"
                onTriggered: triggeredRequestMuteThread()
            }
            MenuSeparator {}
            Action {
                text: qsTr("Report post")
                icon.source: "../images/report.png"
                onTriggered: triggeredRequestReport()
            }
        }
    }
}
