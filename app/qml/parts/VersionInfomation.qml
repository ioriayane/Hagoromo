import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

ColumnLayout {
    Label {
        Layout.alignment: Qt.AlignRight
        font.pointSize: 10
        color: Material.color(Material.Grey)
        text: "羽衣 -Hagoromo-"
    }
    Label {
        Layout.alignment: Qt.AlignRight
        Layout.rightMargin: 5
        font.pointSize: 8
        color: Material.color(Material.Grey)
        text: "Version : " + systemTool.applicationVersion
    }
    Label {
        Layout.alignment: Qt.AlignRight
        Layout.rightMargin: 5
        font.pointSize: 8
        color: Material.color(Material.Grey)
        text: "build on Qt " + systemTool.qtVersion
    }
    Label {
        Layout.alignment: Qt.AlignRight
        Layout.rightMargin: 5
        font.pointSize: 8
        color: Material.color(Material.Grey)
        text: "© 2023 Iori Ayane"
    }
}
