import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "../compat"

ColumnLayout {
    id: tabBar
    spacing: 0

    onCurrentIndexChanged: {
        for(var i=0; i<group.buttons.length; i++){
            if(group.buttons[i].value === currentIndex){
                group.buttons[i].checked = true
            }
        }
    }

    property int currentIndex: -1
    property string profileSource: ""

    function moveIndicator(checkedButton) {
        if(checkedButton === null){
            return
        }
        indicator.x = checkedButton.x
        indicator.y = checkedButton.parent.y + checkedButton.height - indicator.height
        indicator.width = checkedButton.width
    }

    ButtonGroup {
        id: group
        onClicked: (button) => {
            tabBar.currentIndex = button.value
            tabBar.moveIndicator(button)
        }
    }

    RowLayout {
        spacing: 0
        ProfileTabButton {
            id: postButton
            Layout.fillWidth: true
            ButtonGroup.group: group
            value: 0
            source: "../images/reply.png"
            Rectangle {
                id: indicator
                x: parent.x
                y: parent.y + parent.height - height
                width: parent.width
                height: 2
                radius: 1
                color: Material.accentColor
                visible: group.checkedButton != null
                Behavior on x {
                    NumberAnimation { duration: 100 }
                }
                Behavior on y {
                    NumberAnimation { duration: 100 }
                }
                Behavior on width {
                    NumberAnimation { duration: 100 }
                }
            }
            onWidthChanged: {
                if(checked){
                    moveIndicator(postButton)
                }
            }
        }
        ProfileTabButton {
            id: repostButton
            Layout.fillWidth: true
            ButtonGroup.group: group
            value: 1
            source: "../images/repost.png"
            iconColor: Material.color(Material.Green)
            onWidthChanged: {
                if(checked){
                    moveIndicator(repostButton)
                }
            }
        }
        ProfileTabButton {
            id: likeButton
            Layout.fillWidth: true
            ButtonGroup.group: group
            value: 2
            source: "../images/like.png"
            iconColor: Material.color(Material.Pink)
            onWidthChanged: {
                if(checked){
                    moveIndicator(likeButton)
                }
            }
        }
        ProfileTabButton {
            id: mediaButton
            Layout.fillWidth: true
            ButtonGroup.group: group
            value: 3
            source: "../images/media.png"
            iconColor: Material.color(Material.Blue)
            onWidthChanged: {
                if(checked){
                    moveIndicator(mediaButton)
                }
            }
        }
        ProfileTabButton {
            id: videoButton
            Layout.fillWidth: true
            ButtonGroup.group: group
            value: 4
            source: "../images/video.png"
            iconColor: Material.color(Material.Blue)
            onWidthChanged: {
                if(checked){
                    moveIndicator(videoButton)
                }
            }
        }
        ProfileTabButton {
            id: feedButton
            Layout.fillWidth: true
            ButtonGroup.group: group
            value: 5
            source: "../images/feed.png"
            onWidthChanged: {
                if(checked){
                    moveIndicator(feedButton)
                }
            }
        }
        ProfileTabButton {
            id: listButton
            Layout.fillWidth: true
            ButtonGroup.group: group
            value: 6
            source: "../images/list.png"
            onWidthChanged: {
                if(checked){
                    moveIndicator(listButton)
                }
            }
        }
    }
    RowLayout {
        spacing: 0
        ProfileTabButton {
            id: followButton
            Layout.fillWidth: true
            ButtonGroup.group: group
            value: 7
            contentItem: RowLayout {
                spacing: 1
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                }
                AvatarImage {
                    Layout.preferredWidth: AdjustedValues.i16
                    Layout.preferredHeight: AdjustedValues.i16
                    acceptedButtons: Qt.NoButton
                    source: tabBar.profileSource
                }
                Image {
                    Layout.preferredWidth: AdjustedValues.i16
                    Layout.preferredHeight: AdjustedValues.i16
                    layer.enabled: true
                    layer.effect: ColorOverlayC {
                        color: Material.foreground
                    }
                    source: "../images/arrow_forward.png"
                }
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                }
            }
            onWidthChanged: {
                if(checked){
                    moveIndicator(followButton)
                }
            }
        }
        ProfileTabButton {
            id: followedButton
            Layout.fillWidth: true
            ButtonGroup.group: group
            value: 8
            contentItem: RowLayout {
                spacing: 1
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                }
                AvatarImage {
                    Layout.preferredWidth: AdjustedValues.i16
                    Layout.preferredHeight: AdjustedValues.i16
                    acceptedButtons: Qt.NoButton
                    source: tabBar.profileSource
                }
                Image {
                    Layout.preferredWidth: AdjustedValues.i16
                    Layout.preferredHeight: AdjustedValues.i16
                    layer.enabled: true
                    layer.effect: ColorOverlayC {
                        color: Material.foreground
                    }
                    source: "../images/arrow_back.png"
                }
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                }
            }
            onWidthChanged: {
                if(checked){
                    moveIndicator(followedButton)
                }
            }
        }
    }
}
