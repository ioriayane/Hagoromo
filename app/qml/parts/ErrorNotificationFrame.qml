import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import tech.relog.hagoromo.singleton 1.0

import "../controls"

Frame {
    id: errorNotificationFrame

    property string headerText: ""
    property string message: ""

    signal clicked
    signal closeClicked

    contentWidth: frameLayout.width
    contentHeight: frameLayout.height
    background: Rectangle {
        radius: 3
        border.width: 1
        border.color: Material.color(Material.Red, Material.Shade600)
        color: Material.backgroundColor
    }

    RowLayout {
        id: frameLayout
        width: 300 * AdjustedValues.ratio
        spacing: 0

        MouseArea {
            Layout.fillWidth: true
            Layout.preferredHeight: contentLayout.height
            cursorShape: Qt.PointingHandCursor
            onClicked: errorNotificationFrame.clicked()

            ColumnLayout {
                id: contentLayout
                width: parent.width
                spacing: 2

                Label {
                    id: headerLabel
                    Layout.fillWidth: true
                    font.pointSize: AdjustedValues.f10
                    elide: Text.ElideRight
                    maximumLineCount: 1
                    wrapMode: Text.NoWrap
                    text: errorNotificationFrame.headerText.split("\n")[0]
                    clip: true
                }
                Label {
                    id: messageLabel
                    Layout.fillWidth: true
                    font.pointSize: AdjustedValues.f8
                    elide: Text.ElideRight
                    maximumLineCount: 1
                    wrapMode: Text.NoWrap
                    text: errorNotificationFrame.message.split("\n")[0]
                    clip: true
                }
            }
        }
        IconButton {
            flat: true
            Layout.alignment: Qt.AlignTop
            iconSource: "../images/close.png"
            onClicked: errorNotificationFrame.closeClicked()
        }
    }
}
