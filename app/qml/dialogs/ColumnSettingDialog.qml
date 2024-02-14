import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import Qt.labs.settings 1.0

import tech.relog.hagoromo.singleton 1.0

import "../controls"

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
    property alias imageLayoutCombobox: imageLayoutCombobox
    property alias visibleLikeCheckBox: visibleLikeCheckBox
    property alias visibleRepostCheckBox: visibleRepostCheckBox
    property alias visibleFollowCheckBox: visibleFollowCheckBox
    property alias visibleMentionCheckBox: visibleMentionCheckBox
    property alias visibleReplyCheckBox: visibleReplyCheckBox
    property alias visibleQuoteCheckBox: visibleQuoteCheckBox
    property alias visibleReplyToUnfollowedUsersCheckBox: visibleReplyToUnfollowedUsersCheckBox

    ColumnLayout {
        spacing: AdjustedValues.s5
        states: [
            State {
                when: columnSettingDialog.componentType === 0
                PropertyChanges { target: displayFrame; visible: true }
                PropertyChanges { target: visibleReplyToUnfollowedUsersCheckBox; visible: true }
                PropertyChanges { target: imageLayoutLabel; visible: true }
                PropertyChanges { target: imageLayoutCombobox; visible: true }
            },
            State {
                when: columnSettingDialog.componentType === 1
                PropertyChanges { target: displayFrame; visible: true }
                PropertyChanges { target: visibleLikeCheckBox; visible: true }
                PropertyChanges { target: visibleRepostCheckBox; visible: true }
                PropertyChanges { target: visibleFollowCheckBox; visible: true }
                PropertyChanges { target: visibleMentionCheckBox; visible: true }
                PropertyChanges { target: visibleReplyCheckBox; visible: true }
                PropertyChanges { target: visibleQuoteCheckBox; visible: true }
                PropertyChanges { target: imageLayoutLabel; visible: true }
                PropertyChanges { target: imageLayoutCombobox; visible: true }
            },
            State {
                when: columnSettingDialog.componentType === 2
                PropertyChanges { target: imageLayoutLabel; visible: true }
                PropertyChanges { target: imageLayoutCombobox; visible: true }
            },
            State {
                when: columnSettingDialog.componentType === 3
                PropertyChanges { target: imageLayoutLabel; visible: false }
                PropertyChanges { target: imageLayoutCombobox; visible: false }
            },
            State {
                when: columnSettingDialog.componentType === 4
                PropertyChanges { target: imageLayoutLabel; visible: true }
                PropertyChanges { target: imageLayoutCombobox; visible: true }
            },
            State {
                when: columnSettingDialog.componentType === 5
                PropertyChanges { target: imageLayoutLabel; visible: true }
                PropertyChanges { target: imageLayoutCombobox; visible: true }
            },
            State {
                when: columnSettingDialog.componentType === 6
                PropertyChanges { target: imageLayoutLabel; visible: true }
                PropertyChanges { target: imageLayoutCombobox; visible: true }
            }
        ]

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
                    ComboBoxEx {
                        id: autoLoadingIntervalCombo
                        Layout.fillWidth: true
                        Layout.preferredHeight: implicitHeight * AdjustedValues.ratio
                        model: ListModel {
                            ListElement { value: 60000; text: qsTr("1 min.") }
                            ListElement { value: 180000; text: qsTr("3 min.") }
                            ListElement { value: 300000; text: qsTr("5 min.") }
                            ListElement { value: 600000; text: qsTr("10 min.") }
                            ListElement { value: 900000; text: qsTr("15 min.") }
                            ListElement { value: 1200000; text: qsTr("20 min.") }
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
                        Layout.preferredWidth: 175 * AdjustedValues.ratio
                        from: 300
                        to: 500
                        stepSize: 50
                        snapMode: Slider.SnapOnRelease
                    }
                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: columnWidthSlider.value
                    }
                }


                Label {
                    id: imageLayoutLabel
                    Layout.topMargin: 15
                    font.pointSize: AdjustedValues.f10
                    visible: false
                    text: qsTr("Image layout")
                }
                ComboBoxEx {
                    id: imageLayoutCombobox
                    Layout.fillWidth: true
                    Layout.preferredHeight: implicitHeight * AdjustedValues.ratio
                    visible: false
                    model: ListModel {
                        ListElement { value: 0; text: qsTr("Compact") }
                        ListElement { value: 1; text: qsTr("Normal") }
                        ListElement { value: 2; text: qsTr("When one is whole") }
                        ListElement { value: 3; text: qsTr("All whole") }
                    }
                }
            }
            Frame {
                id: displayFrame
                Layout.leftMargin: 10
                Layout.fillHeight: true
                visible: false

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
                    spacing: AdjustedValues.s5

                    Label {
                        Layout.bottomMargin: 5
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Display")
                    }

                    CheckBox {
                        id: visibleLikeCheckBox
                        topPadding: 5
                        bottomPadding: 5
                        visible: false
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Like")
                    }
                    CheckBox {
                        id: visibleRepostCheckBox
                        topPadding: 5
                        bottomPadding: 5
                        visible: false
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Repost")
                    }
                    CheckBox {
                        id: visibleFollowCheckBox
                        topPadding: 5
                        bottomPadding: 5
                        visible: false
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Follow")
                    }
                    CheckBox {
                        id: visibleMentionCheckBox
                        topPadding: 5
                        bottomPadding: 5
                        visible: false
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Mention")
                    }
                    CheckBox {
                        id: visibleReplyCheckBox
                        topPadding: 5
                        bottomPadding: 5
                        visible: false
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Reply")
                    }
                    CheckBox {
                        id: visibleQuoteCheckBox
                        topPadding: 5
                        bottomPadding: 5
                        visible: false
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Quote")
                    }
                    CheckBox {
                        id: visibleReplyToUnfollowedUsersCheckBox
                        topPadding: 5
                        bottomPadding: 5
                        visible: false
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Reply to unfollowed users")
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
