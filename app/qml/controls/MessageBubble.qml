import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15


Label {
    id: label

    textFormat: Text.StyledText
    wrapMode: Text.Wrap

    topPadding: 5
    leftPadding: 5
    rightPadding: 5
    bottomPadding: 5

    property bool fromRight: false

    background: Rectangle {
        id: bubbleBackground
        color: (Material.theme === Material.Light) ?
                   Material.color(Material.Grey, Material.Shade300) :
                   Material.color(Material.Grey, Material.Shade800)
        radius: 10
        width: label.width
        height: label.height

        states: [
            State {
                when: label.fromRight
                AnchorChanges {
                    target: fromRect
                    anchors.right: bubbleBackground.right
                    anchors.left: undefined
                }
            }
        ]

        Rectangle {
            id: fromRect
            width: parent.width / 2
            height: parent.height / 2
            color: parent.color
            anchors.left: parent.left
            anchors.bottom: parent.bottom
        }
    }
}
