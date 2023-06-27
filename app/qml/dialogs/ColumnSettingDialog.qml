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
    property alias enableLikeCheckBox: enableLikeCheckBox
    property alias enableRepostCheckBox: enableRepostCheckBox
    property alias enableFollowCheckBox: enableFollowCheckBox
    property alias enableMentionCheckBox: enableMentionCheckBox
    property alias enableReplyCheckBox: enableReplyCheckBox
    property alias enableQuoteCheckBox: enableQuoteCheckBox

    GridLayout {
        columns: 2

        ColumnLayout {
            Layout.alignment: Qt.AlignTop
            spacing: 0

            CheckBox {
                id: autoLoadingCheckbox
                topPadding: 10
                bottomPadding: 10
                text: qsTr("Auto loading")
            }
            RowLayout {
                Layout.topMargin: 10
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
                Layout.topMargin: 15
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
        }
        Frame {
            Layout.leftMargin: 10

            background: Item {
                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    width: 1
                    border.color: Material.color(Material.Grey)
                    border.width: 1
                    color: "transparent"
                }
            }

            ColumnLayout {
                spacing: 5

                Label {
                    text: qsTr("Display in notifications")
                }

                CheckBox {
                    id: enableLikeCheckBox
                    topPadding: 5
                    bottomPadding: 5
                    text: qsTr("Like")
                }
                CheckBox {
                    id: enableRepostCheckBox
                    topPadding: 5
                    bottomPadding: 5
                    text: qsTr("Repost")
                }
                CheckBox {
                    id: enableFollowCheckBox
                    topPadding: 5
                    bottomPadding: 5
                    text: qsTr("Follow")
                }
                CheckBox {
                    id: enableMentionCheckBox
                    topPadding: 5
                    bottomPadding: 5
                    text: qsTr("Mention")
                }
                CheckBox {
                    id: enableReplyCheckBox
                    topPadding: 5
                    bottomPadding: 5
                    text: qsTr("Reply")
                }
                CheckBox {
                    id: enableQuoteCheckBox
                    topPadding: 5
                    bottomPadding: 5
                    text: qsTr("Quote")
                }
            }
        }

        Button {
            Layout.alignment: Qt.AlignLeft
            flat: true
            text: qsTr("Cancel")
            onClicked: columnSettingDialog.reject()
        }
        Button {
            Layout.alignment: Qt.AlignRight
            text: qsTr("OK")
            onClicked: {

                columnSettingDialog.accept()
            }
        }
    }
}
