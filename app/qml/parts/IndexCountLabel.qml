import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

// "index/count" 形式で位置を示す丸角バッジ
Rectangle {
    id: indexCountLabelFrame
    width: indexCountLabel.contentWidth + 8
    height: indexCountLabel.contentHeight + 4
    radius: 4
    color: Material.color(Material.BlueGrey,
                          Material.theme === Material.Light ? Material.Shade100 : Material.Shade800)
    visible: index > 0 && count > 0

    property int index: 0
    property int count: 0
    property alias fontPointSize: indexCountLabel.font.pointSize

    Label {
        id: indexCountLabel
        anchors.centerIn: parent
        text: indexCountLabelFrame.index + "/" + indexCountLabelFrame.count
    }
}
