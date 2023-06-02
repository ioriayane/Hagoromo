import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import Qt.labs.settings 1.0

Dialog {
    id: columnSettingDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Column settings")

    property int parentWidth: parent.width

    property string columnKey: ""

    property alias autoLoadingCheckbox: autoLoadingCheckbox
    property alias autoLoadingIntervalCombo: autoLoadingIntervalCombo
    property alias columnWidthSlider: columnWidthSlider


    ColumnLayout {

        CheckBox {
            id: autoLoadingCheckbox
            text: qsTr("Auto loading")
        }
        RowLayout {
            Label {
                text: qsTr("Interval")
            }
            ComboBox {
                id: autoLoadingIntervalCombo
                Layout.fillWidth: true
                textRole: "text"
                valueRole: "value"
                model: ListModel {
                    ListElement { value: 60000; text: qsTr("1 min.") }
                    ListElement { value: 180000; text: qsTr("3 min.") }
                    ListElement { value: 300000; text: qsTr("5 min.") }
                    ListElement { value: 600000; text: qsTr("10 min.") }
                    ListElement { value: 900000; text: qsTr("15 min.") }
                    ListElement { value: 1200000; text: qsTr("20 min.") }
                }
                function setByValue(value){
                    for(var i=0; i<model.count; i++){
                        if(model.get(i).value === value){
                            currentIndex = i
                            break
                        }
                    }
                }
            }
        }


        Label {
            text: "Column width"
        }
        RowLayout {
            Slider {
                id: columnWidthSlider
                Layout.fillWidth: true
                from: 300
                to: 500
                stepSize: 50
                snapMode: Slider.SnapOnRelease
            }
            Label {
                text: columnWidthSlider.value
            }
        }

        RowLayout {
            Button {
                flat: true
                text: qsTr("Cancel")
                onClicked: columnSettingDialog.reject()
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            Button {
                text: qsTr("OK")
                onClicked: {

                    columnSettingDialog.accept()
                }
            }
        }
    }
}
