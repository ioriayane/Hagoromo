import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "../compat"

Frame {
    property alias iconSource: iconImage.source
    property alias labelText: label.text
    property alias backgroundColor: backgroundRect.color
    property alias borderWidth: backgroundRect.border.width
    property alias controlButton: controlButton

    topPadding: 5
    bottomPadding: 5

    background: Rectangle {
        id: backgroundRect
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        border.width: 1
        border.color: Material.color(Material.Grey, Material.Shade600)
        color: "transparent"
        radius: 2
    }

    RowLayout {
        Image {
            id: iconImage
            Layout.preferredWidth: AdjustedValues.i16
            Layout.preferredHeight: AdjustedValues.i16
            layer.enabled: true
            layer.effect: ColorOverlayC {
                color: Material.foreground
            }
        }
        Label {
            id: label
            font.pointSize: AdjustedValues.f10
        }
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            height: 1
        }

        IconButton {
            id: controlButton
            Layout.preferredHeight: AdjustedValues.b24
            topInset: 0
            bottomInset: 0
            visible: false
            flat: true
            foreground: Material.accentColor
        }
    }
}
