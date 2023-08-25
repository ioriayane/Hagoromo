import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

RadioButton {
    id: control
    text: " "
    rightPadding: indicator.width + control.spacing * 2

    property alias mainText: mainLabel.text
    property alias description: descriptionLabel.text

    contentItem: ColumnLayout {
        anchors.left: control.indicator.right
        anchors.leftMargin: control.spacing
        spacing: 0
        Label {
            id: mainLabel
            font.pointSize: descriptionLabel.font.pointSize + 1
            font.bold: true
        }
        Label {
            id: descriptionLabel
            color: Material.color(Material.Grey)
            font.pointSize: control.font.pointSize
        }
    }
}
