import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import Qt.labs.settings 1.0

import tech.relog.hagoromo.singleton 1.0

Dialog {
    id: columnSettingDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Column settings")

    property int parentWidth: parent.width

    property string columnKey: ""
    property int componentType: -1

    property alias autoLoadingCheckbox: autoLoadingCheckbox
    property alias autoLoadingIntervalCombo: autoLoadingIntervalCombo
    property alias columnWidthSlider: columnWidthSlider
    property alias visibleLikeCheckBox: visibleLikeCheckBox
    property alias visibleRepostCheckBox: visibleRepostCheckBox
    property alias visibleFollowCheckBox: visibleFollowCheckBox
    property alias visibleMentionCheckBox: visibleMentionCheckBox
    property alias visibleReplyCheckBox: visibleReplyCheckBox
    property alias visibleQuoteCheckBox: visibleQuoteCheckBox

    ColumnLayout {
        spacing: 5 * AdjustedValues.ratio
        RowLayout {
            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                spacing: 0

                CheckBox {
                    id: autoLoadingCheckbox
                    topPadding: 10
                    bottomPadding: 10
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("Auto loading")
                }
                RowLayout {
                    Layout.topMargin: 10
                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Interval")
                    }
                    ComboBox {
                        id: autoLoadingIntervalCombo
                        Layout.fillWidth: true
                        Layout.preferredHeight: implicitHeight * AdjustedValues.ratio
                        font.pointSize: AdjustedValues.f10
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
                        delegate: ItemDelegate {
                            width: parent.width
                            height: implicitHeight * AdjustedValues.ratio
                            font.pointSize: AdjustedValues.f10
                            text: model.text
                            onClicked: autoLoadingIntervalCombo.currentIndex = model.index
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
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("Column width")
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
                id: displayFrame
                Layout.leftMargin: 10
                visible: false

                states: [
                    State {
                        when: columnSettingDialog.componentType === 1
                        PropertyChanges { target: displayFrame; visible: true }
                    }
                ]

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
                    spacing: 5 * AdjustedValues.ratio

                    Label {
                        Layout.bottomMargin: 5
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Display")
                    }

                    CheckBox {
                        id: visibleLikeCheckBox
                        topPadding: 5
                        bottomPadding: 5
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Like")
                    }
                    CheckBox {
                        id: visibleRepostCheckBox
                        topPadding: 5
                        bottomPadding: 5
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Repost")
                    }
                    CheckBox {
                        id: visibleFollowCheckBox
                        topPadding: 5
                        bottomPadding: 5
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Follow")
                    }
                    CheckBox {
                        id: visibleMentionCheckBox
                        topPadding: 5
                        bottomPadding: 5
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Mention")
                    }
                    CheckBox {
                        id: visibleReplyCheckBox
                        topPadding: 5
                        bottomPadding: 5
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Reply")
                    }
                    CheckBox {
                        id: visibleQuoteCheckBox
                        topPadding: 5
                        bottomPadding: 5
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Quote")
                    }
                }
            }
        }

        RowLayout {
            Button {
                Layout.alignment: Qt.AlignLeft
                flat: true
                font.pointSize: AdjustedValues.f10
                text: qsTr("Cancel")
                onClicked: columnSettingDialog.reject()
            }
            Item {
                Layout.fillWidth: true
                height: 1
            }
            Button {
                Layout.alignment: Qt.AlignRight
                font.pointSize: AdjustedValues.f10
                text: qsTr("OK")
                onClicked: {

                    columnSettingDialog.accept()
                }
            }
        }
    }
}
