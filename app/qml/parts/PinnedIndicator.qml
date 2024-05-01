import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.singleton 1.0

Rectangle {
    color: Material.color(Material.Indigo)
    radius: height / 2
    RowLayout {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: parent.height / 2 + 2
        spacing: 2
        Image {
            Layout.preferredWidth: AdjustedValues.i12
            Layout.preferredHeight: AdjustedValues.i12
            layer.enabled: true
            layer.effect: ColorOverlay {
                color: "white"
            }
            source: "../images/pin.png"
        }
        Label {
            Layout.topMargin: 3
            text: qsTr("Pinned post")
            font.pointSize: AdjustedValues.f8
            color: "white"
        }
    }
}
