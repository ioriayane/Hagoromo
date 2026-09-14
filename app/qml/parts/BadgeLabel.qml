import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

// 背景色付きの丸角ラベル
Rectangle {
    id: badgeLabelFrame
    width: badgeLabel.contentWidth + 8
    height: badgeLabel.contentHeight + 4
    radius: 4
    color: Material.color(Material.BlueGrey,
                          Material.theme === Material.Light ? Material.Shade100 : Material.Shade800)

    property alias text: badgeLabel.text
    property alias fontPointSize: badgeLabel.font.pointSize

    Label {
        id: badgeLabel
        anchors.centerIn: parent
    }
}
