import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "../controls"

Dialog {
    id: addPollDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Add poll")
    closePolicy: Popup.NoAutoClose

    property var appliedOptions: []
    property int appliedDuration: 0

    onOpened: {
        if(appliedOptions.length > 0){
            option1TextField.text = appliedOptions[0]
        }
        if(appliedOptions.length > 1){
            option2TextField.text = appliedOptions[1]
        }
        if(appliedOptions.length > 2){
            option3TextField.text = appliedOptions[2]
        }
        if(appliedOptions.length > 3){
            option4TextField.text = appliedOptions[3]
        }
        if(appliedDuration > 0){
            durationComboBox.currentIndex = durationComboBox.seconds.indexOf(appliedDuration)
        }

        appliedOptions = []
        appliedDuration = 0
    }

    onClosed: {
        option1TextField.text = ""
        option2TextField.text = ""
        option3TextField.text = ""
        option4TextField.text = ""
        optionsLayout.optionCount = 2
        durationComboBox.currentIndex = 4
    }

    ColumnLayout {
        id: optionsLayout
        property int optionCount: 2

        function decreaseOption(index) {
            if(optionCount < 3){
                return;
            }
            if(index <= 1){
                // 2->1へ移動
                option1TextField.text = option2TextField.text
            }
            if(index <= 2){
                // 3->2へ移動
                option2TextField.text = option3TextField.text
            }
            if(index <= 3){
                // 4->3へ移動
                option3TextField.text = option4TextField.text
            }
            option4TextField.text = ""
            optionCount = optionCount - 1
        }

        RowLayout {
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            TextField  {
                id: option1TextField
                Layout.fillWidth: true
                selectByMouse: true
                font.pointSize: AdjustedValues.f10
                placeholderText: qsTr("Option 1")
            }
            IconButton {
                visible: optionsLayout.optionCount >= 3
                iconSource: "../images/delete.png"
                focusPolicy: Qt.NoTabFocus
                onClicked: optionsLayout.decreaseOption(1)
            }
        }
        RowLayout {
            TextField  {
                id: option2TextField
                Layout.fillWidth: true
                selectByMouse: true
                font.pointSize: AdjustedValues.f10
                placeholderText: qsTr("Option 2")
            }
            IconButton {
                visible: optionsLayout.optionCount >= 3
                iconSource: "../images/delete.png"
                focusPolicy: Qt.NoTabFocus
                onClicked: optionsLayout.decreaseOption(2)
            }
        }
        RowLayout {
            visible: optionsLayout.optionCount >= 3
            TextField  {
                id: option3TextField
                Layout.fillWidth: true
                selectByMouse: true
                font.pointSize: AdjustedValues.f10
                placeholderText: qsTr("Option 3")
            }
            IconButton {
                visible: optionsLayout.optionCount >= 3
                iconSource: "../images/delete.png"
                focusPolicy: Qt.NoTabFocus
                onClicked: optionsLayout.decreaseOption(3)
            }
        }
        RowLayout {
            visible: optionsLayout.optionCount >= 4
            TextField  {
                id: option4TextField
                Layout.fillWidth: true
                selectByMouse: true
                font.pointSize: AdjustedValues.f10
                placeholderText: qsTr("Option 4")
            }
            IconButton {
                visible: optionsLayout.optionCount >= 3
                iconSource: "../images/delete.png"
                focusPolicy: Qt.NoTabFocus
                onClicked: optionsLayout.decreaseOption(4)
            }
        }
        Button {
            Layout.fillWidth: true
            font.pointSize: AdjustedValues.f10
            text: qsTr("+ Add option")
            visible: !option4TextField.visible
            onClicked: {
                optionsLayout.optionCount = optionsLayout.optionCount + 1
            }
        }
        RowLayout {
            Label {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Duration")
            }
            ComboBox {
                id: durationComboBox
                Layout.fillWidth: true
                model: [
                    qsTr("5 minutes"),
                    qsTr("1 hour"),
                    qsTr("6 hours"),
                    qsTr("12 hours"),
                    qsTr("1 day"),
                    qsTr("3 days"),
                    qsTr("7 days")
                ]
                currentIndex: 4
                property var seconds: [
                    300,
                    3600,
                    21600,
                    43200,
                    86400,
                    259200,
                    604800
                ]
            }
        }
        RowLayout {
            Button {
                flat: true
                font.pointSize: AdjustedValues.f10
                text: qsTr("Remove")
                onClicked: {
                    addPollDialog.appliedOptions = []
                    addPollDialog.appliedDuration = 0
                    addPollDialog.reject()
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Apply")
                enabled: option1TextField.text.length > 0 &&
                         option2TextField.text.length > 0 &&
                         (!option3TextField.visible || option3TextField.text.length > 0) &&
                         (!option4TextField.visible || option4TextField.text.length > 0)
                onClicked: {
                    var options = []
                    options.push(option1TextField.text)
                    options.push(option2TextField.text)
                    if(option3TextField.text.length > 0){
                        options.push(option3TextField.text)
                    }
                    if(option4TextField.text.length > 0){
                        options.push(option4TextField.text)
                    }
                    addPollDialog.appliedOptions = options
                    addPollDialog.appliedDuration = durationComboBox.seconds[durationComboBox.currentIndex]
                    addPollDialog.accept()
                }
            }
        }
    }
}
