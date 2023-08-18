import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

Rectangle {
    id: tagLabel
    implicitWidth: image.width + label.contentWidth + 6
    implicitHeight: image.height + 4
    radius: 2
    color: Material.color(Material.BlueGrey,
                          Material.theme === Material.Light ? Material.Shade100 : Material.Shade800)

    property alias source: image.source
    property alias text: label.text

    Image {
        id: image
        x: 2
        anchors.verticalCenter: tagLabel.verticalCenter
        width: height
        height: label.contentHeight
        source: "../images/label.png"
        layer.enabled: true
        layer.effect: ColorOverlay {
            color: Material.foreground
        }
    }
    Label {
        id: label
        anchors.left: image.right
        anchors.verticalCenter: tagLabel.verticalCenter
    }
}
