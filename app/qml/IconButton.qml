import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Button {
    id: button

    Layout.preferredWidth: buttonContextLayout.width
    Layout.preferredHeight: buttonContextLayout.height

    property string iconSource: ""
    property string iconText: ""

    RowLayout {
        id: buttonContextLayout
        spacing: 0
        Image {
            Layout.preferredWidth: 16
            Layout.preferredHeight: 16
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            Layout.topMargin: 2
            Layout.bottomMargin: 2
            source: button.iconSource
        }
        Text {
            Layout.leftMargin: 0
            Layout.rightMargin: 5
            Layout.topMargin: 2
            Layout.bottomMargin: 2
            visible: button.iconText.length > 0
            text: button.iconText
        }
    }
}
