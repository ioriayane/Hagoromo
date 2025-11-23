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
        reportingOptions.reportTypeButtonGroup.checkState = Qt.Unchecked
    }

    Account {
        id: account
    }

    ColumnLayout {
        id: reportTypeLayout
        Layout.rightMargin: 10
        spacing: 0
        Label {
            Layout.minimumWidth: 400 * AdjustedValues.ratio
            font.pointSize: AdjustedValues.f10
            text: qsTr("Why should this post be reviewed?")
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
                enabled: reportingOptions.reportTypeButtonGroup.checkState === Qt.PartiallyChecked &&
                         !reportingOptions.reporter.running &&
                         reportTextArea.realLength <= 300
                font.pointSize: AdjustedValues.f10
                text: qsTr("Send report")
                onClicked: {
                    reportingOptions.reporter.setAccount(account.uuid)
                    reportingOptions.reporter.reportPost(targetUri, targetCid, reportTextArea.text,
                                        [labelerDidComboBox.currentValue],
                                        reportingOptions.reportTypeButtonGroup.checkedButton.reason)
                }
                BusyIndicator {
                    anchors.fill: parent
                    visible: reportingOptions.reporter.running
                }
            }
        }
    }
}
