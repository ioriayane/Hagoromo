import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import Qt.labs.settings 1.0

Dialog {
    id: settingDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5

    property int parentWidth: parent.width

    property alias settings: settings


    // 行間と文字間も調整できると良いかも


    Settings {
        id: settings
        property int theme: Material.Light
        property color accent: Material.color(Material.Pink)

        Component.onCompleted: {
            setRadioButton(themeButtonGroup.buttons, settings.theme)
            setRadioButton(accentButtonGroup.buttons, settings.accent)
        }

        function setRadioButton(buttons, value){
            for(var i=0; i<buttons.length; i++){
                if(buttons[i].value === value){
                    buttons[i].checked = true
                }
            }
        }
    }


    ButtonGroup {
        id: themeButtonGroup
        buttons: themeRowlayout.children
    }
    ButtonGroup {
        id: accentButtonGroup
        buttons: accentGridLayout.children
    }

    ColumnLayout {
        RowLayout {
            id: themeRowlayout
            Label {
                text: qsTr("Theme") + " : "
            }
            RadioButton {
                property int value: Material.Light
                text: qsTr("Light")
            }
            RadioButton {
                property int value: Material.Dark
                text: qsTr("Dark")
            }
        }
        Label {
            text: qsTr("Accent color") + " : "
        }
        GridLayout {
            id: accentGridLayout
            columns: 9
            columnSpacing: 1
            rowSpacing: 1
            Repeater {
                model: [Material.Red, Material.Pink, Material.Purple, Material.DeepPurple,
                    Material.Indigo, Material.Blue, Material.LightBlue, Material.Cyan,
                    Material.Teal, Material.Green, Material.LightGreen, Material.lime,
                    Material.Yellow, Material.Amber, Material.Orange, Material.DeepOrange,
                    Material.Brown, Material.BlueGrey ]
                Button {
                    id: colorSelectButton
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    topInset: 1
                    leftInset: 1
                    rightInset: 1
                    bottomInset: 1
                    background: Rectangle {
                        color: Material.color(modelData)
                        border.color: colorSelectButton.checked ? Material.foreground : Material.background
                        border.width: 2
                        radius: 5
                    }
                    checkable: true
                    property color value: Material.color(modelData)
                }
            }
        }

//        Label {
//            text: qsTr("Font")
//        }
//        ComboBox {
//            id: fontCombo
//            Layout.preferredWidth: 300
//            model: Qt.fontFamilies()
//            delegate: ItemDelegate {
//                text: modelData
//                width: fontCombo.width
//            }
//        }

        RowLayout {
            Button {
                flat: true
                text: qsTr("Cancel")
                onClicked: settingDialog.reject()
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            Button {
                text: qsTr("OK")
                onClicked: {
                    settings.theme = themeButtonGroup.checkedButton.value
                    settings.accent = accentButtonGroup.checkedButton.value
                    settingDialog.accept()
                }
            }
        }
    }
}
