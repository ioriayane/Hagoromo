import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.singleton 1.0

ColumnLayout {
    id: tabBar
    spacing: 0

    onWidthChanged: tabBar.moveIndicator(group.checkedButton)
    onCurrentIndexChanged: {
        if(currentIndex < 0 || currentIndex >= group.buttons.length){
            return
        }
        group.buttons[currentIndex].checked = true
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
        }
        ProfileTabButton {
            Layout.fillWidth: true
            ButtonGroup.group: group
            value: 1
            source: "../images/repost.png"
            iconColor: Material.color(Material.Green)
        }
        ProfileTabButton {
            Layout.fillWidth: true
            ButtonGroup.group: group
            value: 2
            source: "../images/like.png"
            iconColor: Material.color(Material.Pink)
        }
        ProfileTabButton {
            Layout.fillWidth: true
            ButtonGroup.group: group
            value: 3
            source: "../images/media.png"
            iconColor: Material.color(Material.Blue)
        }
        ProfileTabButton {
            Layout.fillWidth: true
            ButtonGroup.group: group
            value: 4
            source: "../images/feed.png"
        }
    }
    RowLayout {
        spacing: 0
        ProfileTabButton {
            Layout.fillWidth: true
            ButtonGroup.group: group
            value: 5
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
                    layer.effect: ColorOverlay {
                        color: Material.foreground
                    }
                    source: "../images/arrow_forward.png"
                }
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                }
            }
        }
        ProfileTabButton {
            Layout.fillWidth: true
            ButtonGroup.group: group
            value: 6
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
                    layer.effect: ColorOverlay {
                        color: Material.foreground
                    }
                    source: "../images/arrow_back.png"
                }
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                }
            }
        }
    }
}
