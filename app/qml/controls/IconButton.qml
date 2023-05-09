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

    RowLayout {
        id: buttonContextLayout
        anchors.centerIn: parent

        spacing: 0
        Image {
            id: iconImage
            Layout.preferredWidth: button.height * 0.5
            Layout.preferredHeight: button.height * 0.5
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            layer.enabled: true
            layer.effect: ColorOverlay {
                color: Material.foreground
            }
        }
        Label {
            id: iconLabel
            Layout.leftMargin: 0
            Layout.rightMargin: 5
            visible: button.iconText.length > 0
        }
    }
}
