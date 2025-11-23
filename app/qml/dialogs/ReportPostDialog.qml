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

    StateGroup {
        states: [
            State {
                when: reasonGroupComboBox.selectedValue === "Violence"
                PropertyChanges { target: reasonViolenceLayout; visible: true }
            },
            State {
                when: reasonGroupComboBox.selectedValue === "Sexual"
                PropertyChanges { target: reasonSexualLayout; visible: true }
            },
            State {
                when: reasonGroupComboBox.selectedValue === "ChildSafety"
                PropertyChanges { target: reasonChildSafetyLayout; visible: true }
            },
            State {
                when: reasonGroupComboBox.selectedValue === "Harassment"
                PropertyChanges { target: reasonHarassmentLayout; visible: true }
            },
            State {
                when: reasonGroupComboBox.selectedValue === "Misleading"
                PropertyChanges { target: reasonMisleadingLayout; visible: true }
            },
            State {
                when: reasonGroupComboBox.selectedValue === "Rule"
                PropertyChanges { target: reasonRuleLayout; visible: true }
            },
            State {
                when: reasonGroupComboBox.selectedValue === "SelfHarm"
                PropertyChanges { target: reasonSelfHarmLayout; visible: true }
            },
            State {
                when: reasonGroupComboBox.selectedValue === "Other"
                PropertyChanges { target: reasonOtherLayout; visible: true }
            }
        ]
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

        ComboBox {
            id: reasonGroupComboBox
            Layout.fillWidth: true
            font.pointSize: AdjustedValues.f10
            model: reporter.getReasonGroupNameList()
            delegate: ItemDelegate {
                width: reasonGroupComboBox.width
                height: implicitHeight * AdjustedValues.ratio
                font.pointSize: AdjustedValues.f10
                onClicked: reasonGroupComboBox.currentIndex = model.index
                ColumnLayout {
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 3
                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: model.modelData
                    }
                    Label {
                        Layout.leftMargin: 5
                        font.pointSize: AdjustedValues.f8
                        color: Material.color(Material.Grey)
                        text: reporter.getReasonGroupDescription(model.modelData)
                    }
                }
            }
            property string selectedValue: ""   // currentValueの置き換え
            onCurrentValueChanged: {
                console.log("currentText=" + currentText + ", currentValue=" + currentValue)
                selectedValue = reporter.getReasonGroupName(currentValue)
                reportTypeButtonGroup.checkState = Qt.Unchecked
            }
        }

        ColumnLayout {
            id: reasonViolenceLayout
            visible: false
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Animal welfare")
                property int reason: Reporter.ReasonViolenceAnimal
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Threats or incitement")
                property int reason: Reporter.ReasonViolenceThreats
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Graphic violent content")
                property int reason: Reporter.ReasonViolenceGraphicContent
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Glorification of violence")
                property int reason: Reporter.ReasonViolenceGlorification
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Extremist content")
                property int reason: Reporter.ReasonViolenceExtremistContent
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Human trafficking")
                property int reason: Reporter.ReasonViolenceTrafficking
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Other violent content")
                property int reason: Reporter.ReasonViolenceOther
                ButtonGroup.group: reportTypeButtonGroup
            }
        }
        ColumnLayout{
            id: reasonSexualLayout
            visible: false
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Adult sexual abuse content")
                property int reason: Reporter.ReasonSexualAbuseContent
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Non-consensual intimate imagery")
                property int reason: Reporter.ReasonSexualNCII
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Deepfake adult content")
                property int reason: Reporter.ReasonSexualDeepfake
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Animal sexual abuse")
                property int reason: Reporter.ReasonSexualAnimal
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Unlabeled adult content")
                property int reason: Reporter.ReasonSexualUnlabeled
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Other sexual violence content")
                property int reason: Reporter.ReasonSexualOther
                ButtonGroup.group: reportTypeButtonGroup
            }
        }
        ColumnLayout{
            id: reasonChildSafetyLayout
            visible: false
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Child Sexual Abuse Material (CSAM)")
                property int reason: Reporter.ReasonChildSafetyCSAM
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Grooming or predatory behavior")
                property int reason: Reporter.ReasonChildSafetyGroom
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Privacy violation of a minor")
                property int reason: Reporter.ReasonChildSafetyPrivacy
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Minor harassment or bullying")
                property int reason: Reporter.ReasonChildSafetyHarassment
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Other child safety issue")
                property int reason: Reporter.ReasonChildSafetyOther
                ButtonGroup.group: reportTypeButtonGroup
            }
        }
        ColumnLayout{
            id: reasonHarassmentLayout
            visible: false
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Trolling")
                property int reason: Reporter.ReasonHarassmentTroll
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Targeted harassment")
                property int reason: Reporter.ReasonHarassmentTargeted
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Hate speech")
                property int reason: Reporter.ReasonHarassmentHateSpeech
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Doxxing")
                property int reason: Reporter.ReasonHarassmentDoxxing
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Other harassing or hateful content")
                property int reason: Reporter.ReasonHarassmentOther
                ButtonGroup.group: reportTypeButtonGroup
            }
        }
        ColumnLayout{
            id: reasonMisleadingLayout
            visible: false
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Fake account or bot")
                property int reason: Reporter.ReasonMisleadingBot
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Impersonation")
                property int reason: Reporter.ReasonMisleadingImpersonation
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Spam")
                property int reason: Reporter.ReasonMisleadingSpam
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Scam")
                property int reason: Reporter.ReasonMisleadingScam
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("False information about elections")
                property int reason: Reporter.ReasonMisleadingElections
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Other misleading content")
                property int reason: Reporter.ReasonMisleadingOther
                ButtonGroup.group: reportTypeButtonGroup
            }
        }
        ColumnLayout{
            id: reasonRuleLayout
            visible: false
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Hacking or system attacks")
                property int reason: Reporter.ReasonRuleSiteSecurity
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Promoting or selling prohibited items or services")
                property int reason: Reporter.ReasonRuleProhibitedSales
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Banned user returning")
                property int reason: Reporter.ReasonRuleBanEvasion
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Other network rule-breaking")
                property int reason: Reporter.ReasonRuleOther
                ButtonGroup.group: reportTypeButtonGroup
            }
        }
        ColumnLayout{
            id: reasonSelfHarmLayout
            visible: false
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Content promoting or depicting self-harm")
                property int reason: Reporter.ReasonSelfHarmContent
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Eating disorders")
                property int reason: Reporter.ReasonSelfHarmED
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Dangerous challenges or activities")
                property int reason: Reporter.ReasonSelfHarmStunts
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Dangerous substances or drug abuse")
                property int reason: Reporter.ReasonSelfHarmSubstances
                ButtonGroup.group: reportTypeButtonGroup
            }
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Other dangerous content")
                property int reason: Reporter.ReasonSelfHarmOther
                ButtonGroup.group: reportTypeButtonGroup
            }
        }
        ColumnLayout {
            id: reasonOtherLayout
            visible: false
            RadioButton {
                Layout.rightMargin: 30
                font.pointSize: AdjustedValues.f10
                text: qsTr("Other")
                property int reason: Reporter.ReasonOther
                ButtonGroup.group: reportTypeButtonGroup
            }
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
