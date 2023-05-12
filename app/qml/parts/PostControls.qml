import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../controls"

RowLayout {
    spacing: 10

    property alias replyButton: replyButton
    property alias repostButton: repostButton
    property alias repostMenuItem: repostMenuItem
    property alias quoteMenuItem: quoteMenuItem
    property alias likeButton: likeButton

    IconButton {
        id: replyButton
        Layout.preferredHeight: 30
        iconSource: "../images/reply.png"
    }
    IconButton {
        id: repostButton
        Layout.preferredHeight: 30
        iconSource: "../images/repost.png"
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
        Layout.preferredHeight: 30
        iconSource: "../images/like.png"
    }
    Item {
        Layout.fillWidth: true
        height: 1
    }
    IconButton {
        id: moreButton
        Layout.preferredHeight: 30
        iconSource: "../images/more.png"
    }
}
