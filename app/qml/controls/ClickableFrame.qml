import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

Frame {
    id: clickableFrame
    clip: true

    signal clicked(var mouse)

    property string style: "Normal"
    property color borderColor: Material.color(Material.Grey, Material.Shade600)

    contentItem: MouseArea {
        onClicked: (mouse) => clickableFrame.clicked(mouse)
    }

    background: Item {
        Rectangle {
            id: backgroundRect
            states: [
                State {
                    when: clickableFrame.style == "Post"
                    PropertyChanges {
                        target: backgroundRect
                        anchors.leftMargin: 5
                        anchors.rightMargin: 5
                        border.width: 0
                        height: 1
                        color: clickableFrame.borderColor
                        radius: 0
                    }
                    AnchorChanges {
                        target: backgroundRect
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: undefined
                    }
                },
                State {
                    when: clickableFrame.style == "PostConnected"
                    PropertyChanges {
                        target: backgroundRect
                        anchors.leftMargin: 5
                        anchors.rightMargin: 5
                        border.width: 0
                        height: 1
                        color: clickableFrame.borderColor
                        opacity: 0.3
                        radius: 0
                    }
                    AnchorChanges {
                        target: backgroundRect
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: undefined
                    }
                }
            ]
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            border.width: 1
            border.color: clickableFrame.borderColor
            color: "transparent"
            radius: 2
        }
    }
}
