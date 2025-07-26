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
    id: notificationDeclarationDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Allow others to be notified of your posts")

    property string defaultValue: ""

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
        recordOperator.requestNotificationDeclaration()
    }
    onClosed: {
        notificationDeclarationDialog.targetDid = ""
        notificationDeclarationDialog.defaultValue = ""
    }

    RecordOperator {
        id: recordOperator
        onErrorOccured: (code, message) => {
            notificationDeclarationDialog.errorOccured(account.did, code, message)
        }
        onFinished: (success, uri, cid) => {
            if(success){
                notificationDeclarationDialog.defaultValue = notificationDeclarationItemGroup.checkedButton.value
            }
        }
        onFinishedRequestNotificationDeclaration: (success, declaration) => {
            console.log("Request Notification Declaration Finished: " + success + "," + declaration)
            if(success){
                if(declaration.length === 0){
                    notificationDeclarationDialog.defaultValue = "followers"
                } else {
                    notificationDeclarationDialog.defaultValue = declaration
                }
                setRadioButton(notificationDeclarationItemGroup.buttons, notificationDeclarationDialog.defaultValue)
            }
        }
    }

    ButtonGroup {
        id: notificationDeclarationItemGroup
        buttons: notificationDeclarationItemLayout.children
    }

    function setRadioButton(buttons, value){
        for(var i=0; i<buttons.length; i++){
            if(buttons[i].value === value){
                buttons[i].checked = true
            }
        }
    }


    ColumnLayout {
        Label {
            text: qsTr("This feature allows users to receive notifications for your new posts and replies.\nWho do you want to enable this for?")
            font.pointSize: AdjustedValues.f8
            color: Material.color(Material.Grey)
        }

        ColumnLayout {
            id: notificationDeclarationItemLayout
            enabled: !recordOperator.running
            RadioButton {
                property string value: "followers"
                font.pointSize: AdjustedValues.f10
                text: qsTr("Anyone who follows me")
            }
            RadioButton {
                property string value: "mutuals"
                font.pointSize: AdjustedValues.f10
                text: qsTr("Only followers who I follow")
            }
            RadioButton {
                property string value: "none"
                font.pointSize: AdjustedValues.f10
                text: qsTr("No one")
            }
        }

        RowLayout {
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Cancel")
                flat: true
                onClicked: notificationDeclarationDialog.reject()
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                id: applyButton
                enabled: !recordOperator.running && 
                    notificationDeclarationItemGroup.checkedButton &&
                    notificationDeclarationItemGroup.checkedButton.value !== notificationDeclarationDialog.defaultValue
                font.pointSize: AdjustedValues.f10
                highlighted: enabled
                text: qsTr("Apply")
                onClicked: {
                    recordOperator.updateNotificationDeclaration(
                        notificationDeclarationItemGroup.checkedButton.value
                    )
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
