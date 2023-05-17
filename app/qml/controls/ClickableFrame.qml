import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

Frame {
    id: clickableFrame

    signal clicked(var mouse)

    property string style: "Normal"

    background: MouseArea {
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
                        color: Material.color(Material.Grey, Material.Shade600)
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
            border.color: Material.color(Material.Grey, Material.Shade600)
            color: "transparent"
            radius: 2
        }

        onClicked: (mouse) => clickableFrame.clicked(mouse)
    }
}
