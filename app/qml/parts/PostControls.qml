import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import "../controls"

RowLayout {
    spacing: 10

    property bool isReposted: false
    property bool isLiked: false
    property string postUri: ""
    property string handle: ""
    property bool mine: false

    property alias replyButton: replyButton
    property alias repostButton: repostButton
    property alias repostMenuItem: repostMenuItem
    property alias quoteMenuItem: quoteMenuItem
    property alias likeButton: likeButton

    signal triggeredTranslate()
    signal triggeredCopyToClipboard()
    signal triggeredDeletePost()

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
        Layout.preferredHeight: 24
        iconSource: "../images/reply.png"
        iconSize: 16
        foreground: Material.color(Material.Grey)
        flat: true
    }
    IconButton {
        id: repostButton
        Layout.preferredHeight: 24
        iconSource: "../images/repost.png"
        iconSize: 16
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
    }
    IconButton {
        id: likeButton
        Layout.preferredHeight: 24
        iconSource: "../images/like.png"
        iconSize: 16
        foreground: isLiked ? Material.color(Material.Pink) : Material.color(Material.Grey)
        flat: true
    }
    Item {
        Layout.fillWidth: true
        height: 1
    }
    IconButton {
        id: moreButton
        Layout.preferredHeight: 24
        iconSource: "../images/more.png"
        iconSize: 16
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
            MenuItem {
                icon.source: "../images/language.png"
                text: qsTr("Translate")
                onTriggered: triggeredTranslate()
            }
            MenuItem {
                icon.source: "../images/copy.png"
                text: qsTr("Copy post text")
                onTriggered: triggeredCopyToClipboard()
            }
            MenuItem {
                text: qsTr("Open in Official")
                enabled: postUri.length > 0 && handle.length > 0
                icon.source: "../images/open_in_other.png"
                onTriggered: openInOhters(postUri, handle)
            }
            MenuSeparator {}
            MenuItem {
                text: qsTr("Delete post")
                enabled: mine
                icon.source: "../images/delete.png"
                onTriggered: triggeredDeletePost()
            }
        }
        Menu {
            id: theirMorePopup
            MenuItem {
                icon.source: "../images/language.png"
                text: qsTr("Translate")
                onTriggered: triggeredTranslate()
            }
            MenuItem {
                icon.source: "../images/copy.png"
                text: qsTr("Copy post text")
                onTriggered: triggeredCopyToClipboard()
            }
            MenuItem {
                text: qsTr("Open in Official")
                enabled: postUri.length > 0 && handle.length > 0
                icon.source: "../images/open_in_other.png"
                onTriggered: openInOhters(postUri, handle)
            }
        }
    }
}
