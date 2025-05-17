import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

Rectangle {
    border.width: 2
    border.color: Material.accentColor
    color: "transparent"
    radius: width / 2
    Rectangle {
        anchors.fill: parent
        anchors.margins: 2
        border.width: 2
        border.color: Material.backgroundColor
        color: "transparent"
        radius: width / 2
    }
    Label {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 2
        font.pointSize: AdjustedValues.f6
        text: "LIVE"
        Rectangle {
            anchors.fill: parent
            anchors.leftMargin: -2
            anchors.rightMargin: -2
            z: -1
            color: Material.accentColor
        }
    }
}
