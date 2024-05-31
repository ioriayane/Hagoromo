import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "../compat"

RowLayout {
    id: reactionAuthor
    clip: true

    property alias source: iconImage.source
    property alias displayName: displayNameLabel.text
    property alias handle: handleLabel.text
    property color color: Material.color(Material.Grey)

    Image {
        id: iconImage
        Layout.preferredWidth: AdjustedValues.i12
        Layout.preferredHeight: AdjustedValues.i12
        layer.enabled: true
        layer.effect: ColorOverlayC {
            color: reactionAuthor.color
        }
    }
    Label {
        id: displayNameLabel
        Layout.alignment: Qt.AlignCenter
//        Layout.maximumWidth: reactionAuthor.width - iconImage.width - handleLabel.width - reactionAuthor.spacing * 2
        font.pointSize: AdjustedValues.f8
        color: reactionAuthor.color
        elide: Text.ElideRight
    }
    Label {
        id: handleLabel
        Layout.alignment: Qt.AlignCenter
        Layout.fillWidth: true
        font.pointSize: AdjustedValues.f8
        opacity: 0.8
        color: reactionAuthor.color
    }
}
