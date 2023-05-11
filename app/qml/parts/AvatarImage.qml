import QtQuick 2.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import "../controls"

ImageWithIndicator {
    id: avatorImage
    layer.effect: ColorOverlay {
        color: Material.color(Material.Grey)
    }
    states: [
        State {
            when: model.avatar.length === 0
            PropertyChanges {
                target: avatorImage
                source: "../images/account_icon.png"
                layer.enabled: true
            }
        }
    ]
}
