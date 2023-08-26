import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

ColumnLayout {
    Label {
        Layout.alignment: Qt.AlignRight
        font.pointSize: AdjustedValues.f10
        color: Material.color(Material.Grey)
        text: "羽衣 -Hagoromo-"
    }
    Label {
        Layout.alignment: Qt.AlignRight
        Layout.rightMargin: 5
        font.pointSize: AdjustedValues.f8
        color: Material.color(Material.Grey)
        text: "Version : " + systemTool.applicationVersion
    }
    Label {
        Layout.alignment: Qt.AlignRight
        Layout.rightMargin: 5
        font.pointSize: AdjustedValues.f8
        color: Material.color(Material.Grey)
        text: "build on Qt " + systemTool.qtVersion
    }
    Label {
        Layout.alignment: Qt.AlignRight
        Layout.rightMargin: 5
        font.pointSize: AdjustedValues.f8
        color: Material.color(Material.Grey)
        text: "© 2023 Iori Ayane"
    }
}
