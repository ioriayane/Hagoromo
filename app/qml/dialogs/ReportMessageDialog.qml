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
        reportTextArea.text = ""
        reportingOptions.reset()
    }

    Account {
        id: account
    }
    SystemTool {
        id: systemTool
    }

    ColumnLayout {
        id: reportTypeLayout
        Layout.rightMargin: 20
        spacing: 0

        Label {
            Layout.minimumWidth: 400 * AdjustedValues.ratio
            font.pointSize: AdjustedValues.f10
            text: qsTr("Why should this message be reviewed?")
        }

        ReportingOptions {
            id: reportingOptions
            Layout.fillWidth: true
            reporter.onFinished: (success) => reportDialog.accept()
            reporter.onErrorOccured: (code, message) => reportDialog.errorOccured(reportDialog.account.uuid, code, message)
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
                enabled: reportingOptions.reportTypeButtonGroup.checkState === Qt.PartiallyChecked &&
                         !reportingOptions.reporter.running &&
                         reportTextArea.realLength <= 300
                font.pointSize: AdjustedValues.f10
                text: qsTr("Send report")
                onClicked: {
                    reportingOptions.reporter.setAccount(account.uuid)
                    reportingOptions.reporter.reportMessage(targetAccountDid, targetConvoId, targetMessageId,
                                           reportTextArea.text, reportingOptions.reportTypeButtonGroup.checkedButton.reason)
                }
                BusyIndicator {
                    anchors.fill: parent
                    visible: reportingOptions.reporter.running
                }
            }
        }
    }
}
