import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.reporter 1.0
import tech.relog.hagoromo.singleton 1.0
import tech.relog.hagoromo.systemtool 1.0

import "../controls"
import "../data"
import "../parts"

Dialog {
    id: reportDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5

    title: qsTr("Report message")

    property string targetAccountDid: ""
    property string targetConvoId: ""
    property string targetMessageId: ""
    property alias account: account
    signal errorOccured(string account_uuid, string code, string message)

    onClosed: {
        reportTypeButtonGroup.checkState = Qt.Unchecked
        reportTextArea.text = ""
    }

    Account {
        id: account
    }
    Reporter {
        id: reporter
        onFinished: (success) => reportDialog.accept()
        onErrorOccured: (code, message) => reportDialog.errorOccured(reportDialog.account.uuid, code, message)
    }
    SystemTool {
        id: systemTool
    }

    ButtonGroup {
        id: reportTypeButtonGroup
    }

    ColumnLayout {
        id: reportTypeLayout
        Layout.rightMargin: 20
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
            text: qsTr("Why should this message be reviewed?")
        }

        RadioButtonEx {
            Layout.rightMargin: 20
            Layout.topMargin: 5
            font.pointSize: AdjustedValues.f10
            mainText: qsTr("Spam")
            description: qsTr("Excessive or unwanted messages")
            property int reason: Reporter.ReasonSpam
            ButtonGroup.group: reportTypeButtonGroup
        }
        RadioButtonEx {
            Layout.rightMargin: 20
            font.pointSize: AdjustedValues.f10
            mainText: qsTr("Unwanted Sexual Content")
            description: qsTr("Inappropriate messages or explicit links")
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
                    reporter.reportMessage(targetAccountDid, targetConvoId, targetMessageId,
                                           reportTextArea.text, reportTypeButtonGroup.checkedButton.reason)
                }
                BusyIndicator {
                    anchors.fill: parent
                    visible: reporter.running
                }
            }
        }
    }
}
