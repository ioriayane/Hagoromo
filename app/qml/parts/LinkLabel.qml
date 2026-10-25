import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import "../compat"

Rectangle {
    id: linkLabel
    implicitWidth: image.width + label.contentWidth + spacing + height
    implicitHeight: label.contentHeight + 6
    radius: height / 2
    color: highlight ? Material.color(Material.Teal,
                                      Material.theme === Material.Light ? Material.Shade100 : Material.Shade700)
                     : Material.color(Material.BlueGrey,
                                      Material.theme === Material.Light ? Material.Shade100 : Material.Shade800)

    property string url: ""
    property string iconSource: ""
    property alias text: label.text
    property alias fontPointSize: label.font.pointSize
    property int spacing: 3
    property bool highlight: false
    property alias hovered: mouseArea.containsMouse

    signal clicked(string url)

    Image {
        id: image
        x: linkLabel.height / 2
        anchors.verticalCenter: linkLabel.verticalCenter
        width: height
        height: label.contentHeight
        fillMode: Image.PreserveAspectFit
        source: linkLabel.iconSource.length > 0 ? linkLabel.iconSource : "../images/link.png"
        layer.enabled: linkLabel.iconSource.length === 0
        layer.effect: ColorOverlayC {
            color: Material.foreground
        }
    }
    Label {
        id: label
        anchors.left: image.right
        anchors.leftMargin: linkLabel.spacing
        anchors.verticalCenter: linkLabel.verticalCenter
    }
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: linkLabel.clicked(linkLabel.url)
    }
}
