import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.singleton 1.0

TabButton {
    id: button
    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    implicitHeight: AdjustedValues.b24
    checkable: true
    contentItem: Item {
        Image {
            id: image
            width: AdjustedValues.i16
            height: AdjustedValues.i16
            anchors.centerIn: parent
            layer.enabled: true
            layer.effect: ColorOverlay {
                color: button.iconColor
            }
            source: button.source
        }
    }
    property int value: 0
    property string source: ""
    property var iconColor: Material.foreground
}
