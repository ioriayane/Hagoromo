import QtQuick 2.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import "../controls"

ImageWithIndicator {
    id: avatorImage
    Image {
        visible: avatorImage.status === Image.Null
        anchors.fill: parent
        layer.enabled: true
        layer.effect: ColorOverlay {
            color: Material.color(Material.Grey)
        }
        source: "../images/account_icon.png"
    }
}
