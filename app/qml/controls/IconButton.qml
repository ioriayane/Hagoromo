import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

Button {
    id: button

    Layout.preferredWidth: buttonContextLayout.width

    property string iconSource: ""
    property string iconText: ""

    RowLayout {
        id: buttonContextLayout
        anchors.centerIn: parent

        spacing: 0
        Image {
            Layout.preferredWidth: button.height * 0.5
            Layout.preferredHeight: button.height * 0.5
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            source: button.iconSource
            layer.enabled: true
            layer.effect: ColorOverlay {
                color: Material.foreground
            }
        }
        Label {
            Layout.leftMargin: 0
            Layout.rightMargin: 5
            visible: button.iconText.length > 0
            text: button.iconText
        }
    }
}
