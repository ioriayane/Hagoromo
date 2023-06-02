import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

Button {
    id: button

    Layout.preferredWidth: buttonContextLayout.width

    property alias iconSource: iconImage.source
    property alias iconText: iconLabel.text
    property int iconSize: button.height * 0.5
    property color foreground: Material.foreground


    topInset: 2
    leftInset: 2
    rightInset: 2
    bottomInset: 2

    RowLayout {
        id: buttonContextLayout
        anchors.centerIn: parent
        spacing: 0

        Image {
            id: iconImage
            visible: status !== Image.Null
            Layout.preferredWidth: iconSize
            Layout.preferredHeight: iconSize
            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            layer.enabled: true
            layer.effect: ColorOverlay {
                color: button.foreground
            }
        }
        Label {
            id: iconLabel
            Layout.leftMargin: iconImage.visible ? 0 : 20
            Layout.rightMargin: iconImage.visible ? 5 : 20
            visible: button.iconText.length > 0
            color: button.foreground
        }
    }
}
