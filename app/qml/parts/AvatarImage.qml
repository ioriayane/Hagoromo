import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import "../controls"

Item {
    clip: true
    property alias source: avatorImage.source
    Image {
        id: avatorImage
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
        visible: false
    }
    Image {
        id: mask
        anchors.fill: parent
        source: "../images/icon_mask.png"
        visible: false
    }
    OpacityMask {
        anchors.fill: parent
        source: avatorImage
        maskSource: mask
    }
    BusyIndicator {
        anchors.centerIn: parent
        width: parent.width > 100 ? 100 : parent.width
        height: parent.height > 100 ? 100 : parent.height

        visible: avatorImage.status === Image.Loading ||
                 avatorImage.status === Image.Error
    }
    Image {
        id: altImage
        visible: avatorImage.status === Image.Null
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
        layer.enabled: true
        layer.effect: ColorOverlay {
            color: Material.color(Material.Grey)
        }
        source: "../images/account_icon.png"
    }
}
