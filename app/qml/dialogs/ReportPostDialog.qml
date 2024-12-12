import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.moderation.labelerlistmodel 1.0
import tech.relog.hagoromo.reporter 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"

Dialog {
    id: reportDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5

    title: qsTr("Report post")

    property string targetUri: ""
    property string targetCid: ""
    property alias account: account
    signal errorOccured(string account_uuid, string code, string message)

    onOpened: labelerDidComboBox.load()
    onClosed: {
        reportTextArea.text = ""
        reportTypeButtonGroup.checkState = Qt.Unchecked
    }

    Account {
        id: account
    }
    Reporter {
        id: reporter
        onFinished: (success) => reportDialog.accept()
        onErrorOccured: (code, message) => reportDialog.errorOccured(reportDialog.account.uuid, code, message)
    }

    ButtonGroup {
        id: reportTypeButtonGroup
    }

    ColumnLayout {
        id: reportTypeLayout
        Layout.rightMargin: 10
        spacing: 0
        //        RowLayout {
        //            AvatarImage {
        //                Layout.preferredWidth: 24
        //                Layout.preferredHeight: 24
        //                source: account.avatar
        //            }
        //            Label {
        //                text: account.handle
        //            }
        //        }

        Label {
            font.pointSize: AdjustedValues.f10
            text: qsTr("Why should this post be reviewed?")
        }

        RadioButtonEx {
            Layout.topMargin: 5
            Layout.rightMargin: 20
            font.pointSize: AdjustedValues.f10
            mainText: qsTr("Spam")
            description: qsTr("Excessive mentions or replies")
            property int reason: Reporter.ReasonSpam
            ButtonGroup.group: reportTypeButtonGroup
        }
        RadioButtonEx {
            Layout.rightMargin: 20
            font.pointSize: AdjustedValues.f10
            mainText: qsTr("Unwanted Sexual Content")
            description: qsTr("Nudity or pornography not labeled as such")
            property int reason: Reporter.ReasonSexual
            ButtonGroup.group: reportTypeButtonGroup
        }
        RadioButtonEx {
            Layout.rightMargin: 20
            font.pointSize: AdjustedValues.f10
            mainText: qsTr("Anti-Social Behavior")
            description: qsTr("Harassment, trolling, or intolerance")
            property int reason: Reporter.ReasonRude
            ButtonGroup.group: reportTypeButtonGroup
        }
        RadioButtonEx {
            Layout.rightMargin: 20
            font.pointSize: AdjustedValues.f10
            mainText: qsTr("Illegal and Urgent")
            description: qsTr("Glaring violations of law or terms of service")
            property int reason: Reporter.ReasonViolation
            ButtonGroup.group: reportTypeButtonGroup
        }
        RadioButtonEx {
            Layout.rightMargin: 20
            font.pointSize: AdjustedValues.f10
            mainText: qsTr("Other")
            description: qsTr("An issue not included in these options")
            property int reason: Reporter.ReasonOther
            ButtonGroup.group: reportTypeButtonGroup
        }



        Label {
            Layout.topMargin: 5
            font.pointSize: AdjustedValues.f10
            text: qsTr("Select the moderation service to report to")
        }
        LabelerComboBox {
            id: labelerDidComboBox
            Layout.fillWidth: true
            service: account.service
            handle: account.handle
            accessJwt: account.accessJwt

            onCurrentValueChanged: {
                console.log("currentText=" + currentText + ", currentValue=" + currentValue)
                // if(currentValue){
                //     contentFilterSettingListModel.labelerDid = currentValue
                // }
            }
            onErrorOccured: (code, message) => reportDialog.errorOccured(reportDialog.account.uuid, code, message)
        }

        Label {
            Layout.topMargin: 5
            font.pointSize: AdjustedValues.f10
            text: qsTr("Optionally provide additional information below:")
        }
        ScrollView {
            Layout.preferredWidth: reportTypeLayout.width
            Layout.preferredHeight: AdjustedValues.v72
            TextArea {
                id: reportTextArea
                property int realLength: systemTool.countText(reportTextArea.text)
                wrapMode: TextInput.WordWrap
                selectByMouse: true
                font.pointSize: AdjustedValues.f10
            }
        }
        Label {
            Layout.alignment: Qt.AlignRight
            font.pointSize: AdjustedValues.f8
            text: (300 - reportTextArea.realLength)
        }

        RowLayout {
            Button {
                Layout.alignment: Qt.AlignLeft
                flat: true
                font.pointSize: AdjustedValues.f10
                text: qsTr("Cancel")
                onClicked: reportDialog.close()
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            Button {
                Layout.alignment: Qt.AlignRight
                enabled: reportTypeButtonGroup.checkState === Qt.PartiallyChecked &&
                         !reporter.running &&
                         reportTextArea.realLength <= 300
                font.pointSize: AdjustedValues.f10
                text: qsTr("Send report")
                onClicked: {
                    reporter.setAccount(account.uuid)
                    reporter.reportPost(targetUri, targetCid, reportTextArea.text,
                                        [labelerDidComboBox.currentValue],
                                        reportTypeButtonGroup.checkedButton.reason)
                }
                BusyIndicator {
                    anchors.fill: parent
                    visible: reporter.running
                }
            }
        }
    }
}
