import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../controls"

RowLayout {
    spacing: 10

    property alias replyButton: replyButton
    property alias repostButton: repostButton
    property alias repostMenuButton: repostMenuButton
    property alias quoteMenuButton: quoteMenuButton
    property alias likeButton: likeButton

    IconButton {
        id: replyButton
        Layout.preferredHeight: 30
        iconSource: "../images/reply.png"
        onClicked: clickedReply()
    }
    IconButton {
        id: repostButton
        Layout.preferredHeight: 30
        iconSource: "../images/repost.png"
        onClicked: {
            var point = repostButton.mapToItem(parent, repostButton.pressX, repostButton.pressY)
            popup.x = point.x - popup.width
            popup.y = point.y
            popup.open()
        }
        Popup {
            id: popup
            width: popupItemLayout.width
            height: popupItemLayout.height
            padding: 0
            ColumnLayout {
                id: popupItemLayout
                spacing: 0
                ItemDelegate {
                    id: repostMenuButton
                    Layout.fillWidth: true
                    text: qsTr("Repost")
                    onClicked: popup.close()
                }
                ItemDelegate {
                    id: quoteMenuButton
                    Layout.fillWidth: true
                    text: qsTr("Quote")
                    onClicked: popup.close()
                }
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
