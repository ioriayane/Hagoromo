import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

Dialog {
    id: root
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5

    property int parentWidth: parent.width

    ColumnLayout {
        Label {
            text: qsTr("Theme")
        }
        RowLayout {
            Rectangle {
                Layout.preferredWidth: themeLightButton.width + 5
                Layout.preferredHeight: themeLightButton.height + 5
                color: "white"
                Button {
                    id: themeLightButton
                    anchors.centerIn: parent
                    Material.theme: Material.Light
                    text: qsTr("Light")
                }
            }
            Rectangle {
                Layout.preferredWidth: themeDarkButton.width + 5
                Layout.preferredHeight: themeDarkButton.height + 5
                color: "black"
                Button {
                    id: themeDarkButton
                    anchors.centerIn: parent
                    Material.theme: Material.Dark
                    text: qsTr("Dark")
                }
            }
        }
        Label {
            text: qsTr("Font")
        }
        ComboBox {
            id: fontCombo
            Layout.preferredWidth: 300
            model: Qt.fontFamilies()
            delegate: ItemDelegate {
                text: modelData
                width: fontCombo.width
            }
        }
    }
}
