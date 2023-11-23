import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15
import Qt.labs.platform 1.1 as P

import tech.relog.hagoromo.recordoperator 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"

Dialog {
    id: editProfileDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Edit my profile")
    closePolicy: Popup.NoAutoClose

    property string avatar: ""
    property string banner: ""
    property string displayName: ""
    property string description: ""

    property alias account: account
    Account {
        id: account
    }
    signal errorOccured(string account_uuid, string code, string message)

    onOpened: {
        if(account.service.length === 0){
            return
        }
        recordOperator.setAccount(account.service, account.did, account.handle,
                                  account.email, account.accessJwt, account.refreshJwt)
        avatarImage.source = editProfileDialog.avatar
        bannerImage.source = editProfileDialog.banner
        displayNameText.text = editProfileDialog.displayName
        descriptionText.text = editProfileDialog.description
    }
    onClosed: {
        displayNameText.clear()
        descriptionText.clear()
        avatarImage.source = "../images/edit.png"
        bannerImage.source = "../images/edit.png"
    }

    Shortcut {  // Close
        // DialogのclosePolicyでEscで閉じられるけど、そのうち編集中の確認ダイアログを
        // 入れたいので別でイベント処理をする。onClosedで閉じるをキャンセルできなさそうなので。
        enabled: editProfileDialog.visible && !addButton.enabled
        sequence: "Esc"
        onActivated: editProfileDialog.close()
    }

    SystemTool {
        id: systemTool
    }
    RecordOperator {
        id: recordOperator
        onFinished: (success) => {
                        if(success){
                            editProfileDialog.accept()
                        }
                    }
    }

    ColumnLayout {
        spacing: 5
        RowLayout {
            Label {
                font.pointSize: AdjustedValues.f10
                text: account.handle
                elide: Text.ElideRight
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
        }

        ImageWithIndicator {
            id: bannerImage
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            fillMode: Image.PreserveAspectCrop
            source: "../images/edit.png"
            MouseArea {
                anchors.fill: parent
                onClicked: (mouse) => {
                               imageClipDialog.target = "banner"
                               fileDialog.open()
                           }
            }
        }


        Rectangle {
            Layout.preferredWidth: AdjustedValues.i24
            Layout.preferredHeight: AdjustedValues.i24
            color: "white"
            radius: width / 2
            AvatarImage {
                id: avatarImage
                anchors.fill: parent
                anchors.margins: 1
                source: "../images/edit.png"
                onClicked: (mouse) => {
                               imageClipDialog.target = "avatar"
                               fileDialog.open()
                           }
            }
        }

        Label {
            Layout.fillWidth: true
            Layout.topMargin: 5
            font.pointSize: AdjustedValues.f10
            text: qsTr("Display Name")
        }
        TextField  {
            id: displayNameText
            Layout.preferredWidth: 300 * AdjustedValues.ratio
            enabled: !recordOperator.running
            selectByMouse: true
            font.pointSize: AdjustedValues.f10
        }

        Label {
            Layout.fillWidth: true
            Layout.topMargin: 5
            font.pointSize: AdjustedValues.f10
            text: qsTr("Description")
        }
        ScrollView {
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            Layout.preferredHeight: 120 * AdjustedValues.ratio
            TextArea {
                id: descriptionText
                verticalAlignment: TextInput.AlignTop
                enabled: !recordOperator.running
                wrapMode: TextInput.WordWrap
                selectByMouse: true
                font.pointSize: AdjustedValues.f10
                property int realTextLength: systemTool.countText(text)
            }
        }

        Label {
            Layout.alignment: Qt.AlignRight
            font.pointSize: AdjustedValues.f8
            text: 300 - descriptionText.realTextLength
        }

        RowLayout {
            Button {
                Layout.alignment: Qt.AlignLeft
                flat: true
                font.pointSize: AdjustedValues.f10
                text: qsTr("Cancel")
                onClicked: editProfileDialog.reject()
            }
            Item {
                Layout.fillWidth: true
                height: 1
            }
            Button {
                id: addButton
                Layout.alignment: Qt.AlignRight
                font.pointSize: AdjustedValues.f10
                enabled: displayNameText.text.length > 0 && descriptionText.realTextLength <= 300 && !recordOperator.running
                text: qsTr("Add")
                onClicked: {
                    recordOperator.clear()
                    recordOperator.updateProfile(avatarImage.source, bannerImage.source,
                                                 displayNameText.text, descriptionText.text)
                }
                BusyIndicator {
                    anchors.fill: parent
                    anchors.margins: 3
                    visible: recordOperator.running
                }
            }
        }
    }

    P.FileDialog {
        id: fileDialog
        title: qsTr("Select contents")
        visible: false
        fileMode : P.FileDialog.OpenFiles

        nameFilters: ["Image files (*.jpg *.jpeg *.png)"
            , "All files (*)"]
        onAccepted: {
            prevFolder = folder
            imageClipDialog.embedImage = file
            imageClipDialog.open()
        }
        property string prevFolder
    }
    ImageClipDialog {
        id: imageClipDialog
        property string target: ""
        onRejected: embedImage = ""
        onAccepted: {
            console.log("Selected=" + selectedX + "," + selectedY + "," + selectedWidth + "," + selectedHeight)
            var cliped = systemTool.clipImage(embedImage,
                                              selectedX, selectedY,
                                              selectedWidth, selectedHeight)
            if(target === "avatar"){
                avatarImage.source = cliped
            }else if(target === "banner"){
                bannerImage.source = cliped
            }
        }
    }
}
