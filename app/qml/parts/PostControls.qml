import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import "../controls"

RowLayout {
    spacing: 10

    property alias replyButton: replyButton
    property alias repostButton: repostButton
    property alias repostMenuItem: repostMenuItem
    property alias quoteMenuItem: quoteMenuItem
    property alias likeButton: likeButton

    property alias tranlateMenuItem: tranlateMenuItem

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
        foreground: Material.color(Material.Grey)
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
                icon.source: "../images/reply.png"
                text: qsTr("Quote")
            }
        }
    }
    IconButton {
        id: likeButton
        Layout.preferredHeight: 24
        iconSource: "../images/like.png"
        iconSize: 16
        foreground: Material.color(Material.Grey)
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
        onClicked: morePopup.open()
        Menu {
            id: morePopup
            MenuItem {
                id: tranlateMenuItem
                text: qsTr("Translate")
            }
        }
    }
}
