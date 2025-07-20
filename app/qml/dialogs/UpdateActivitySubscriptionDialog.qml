import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.recordoperator 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"

Dialog {
    id: updateActivitySubscriptionDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Keep me posted")

    property string targetDid: ""
    property bool defaultPost: false
    property bool defaultReply: false

    property alias account: account
    Account {
        id: account
    }
    signal errorOccured(string account_uuid, string code, string message)

    onOpened: {
        if(account.uuid.length === 0){
            return
        }
        recordOperator.setAccount(account.uuid)
        postSwitch.checked = updateActivitySubscriptionDialog.defaultPost
        replySwitch.checked = updateActivitySubscriptionDialog.defaultReply
    }
    onClosed: {
        updateActivitySubscriptionDialog.targetDid = ""
        postSwitch.checked = false
        replySwitch.checked = false
    }

    RecordOperator {
        id: recordOperator
        onErrorOccured: (code, message) => {
            updateActivitySubscriptionDialog.errorOccured(account.did, code, message)
        }
        onFinished: (success, uri, cid) => {
            if(success){
                updateActivitySubscriptionDialog.defaultPost = postSwitch.checked
                updateActivitySubscriptionDialog.defaultReply = replySwitch.checked
            }
        }
    }

    ColumnLayout {
        Label {
            text: qsTr("Get notified of this account’s activity")
            font.pointSize: AdjustedValues.f8
            color: Material.color(Material.Grey)
        }

        Switch {
            id: postSwitch
            text: qsTr("Post")
            font.pointSize: AdjustedValues.f10
            onCheckedChanged: {
                if(!checked){
                    replySwitch.checked = false
                }
            }
        }
        Switch {
            id: replySwitch
            text: qsTr("Reply")
            enabled: postSwitch.checked
            font.pointSize: AdjustedValues.f10
        }

        RowLayout {
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Cancel")
                flat: true
                onClicked: updateActivitySubscriptionDialog.reject()
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                id: applyButton
                enabled: !recordOperator.running && (
                    postSwitch.checked !== updateActivitySubscriptionDialog.defaultPost ||
                    replySwitch.checked !== updateActivitySubscriptionDialog.defaultReply
                )
                font.pointSize: AdjustedValues.f10
                highlighted: enabled
                text: qsTr("Apply")
                onClicked: {
                    recordOperator.updateActivitySubscription(updateActivitySubscriptionDialog.targetDid
                                                                , postSwitch.checked, replySwitch.checked)
                }
                BusyIndicator {
                    anchors.fill: parent
                    anchors.margins: 3
                    visible: recordOperator.running
                }
            }
        }
    }
}
