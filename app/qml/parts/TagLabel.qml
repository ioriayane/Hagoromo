import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

Rectangle {
    id: tagLabel
    implicitWidth: image.width + label.contentWidth + label.anchors.leftMargin + 6
    implicitHeight: image.height + 4
    radius: 2
    color: Material.color(Material.BlueGrey,
                          Material.theme === Material.Light ? Material.Shade100 : Material.Shade800)

    property alias source: image.source
    property alias text: label.text
    property alias fontPointSize: label.font.pointSize
    property int spacing: 0

    Image {
        id: image
        x: 2
        anchors.verticalCenter: tagLabel.verticalCenter
        width: status !== Image.Null ? height: 0
        height: label.contentHeight
        visible: status !== Image.Null
        source: "../images/label.png"
        layer.enabled: true
        layer.effect: ColorOverlay {
            color: Material.foreground
        }
    }
    Label {
        id: label
        anchors.left: image.right
        anchors.leftMargin: tagLabel.spacing
        anchors.verticalCenter: tagLabel.verticalCenter
    }
}
