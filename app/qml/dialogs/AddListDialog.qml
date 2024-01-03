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
    id: addListDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: editMode ? qsTr("Edit a list") : qsTr("Add a list")
    closePolicy: Popup.NoAutoClose

    property bool editMode: false
    property string listUri: ""
    property string avatar: ""
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
        if(addListDialog.editMode){
            if(addListDialog.avatar.length > 0){
                avatarImage.source = addListDialog.avatar
            }
            nameText.text = addListDialog.displayName
            descriptionText.text = addListDialog.description
        }
    }
    onClosed: {
        addListDialog.editMode = false
        nameText.clear()
        descriptionText.clear()
        avatarImage.source = "../images/edit.png"
    }

    Shortcut {  // Close
        // DialogのclosePolicyでEscで閉じられるけど、そのうち編集中の確認ダイアログを
        // 入れたいので別でイベント処理をする。onClosedで閉じるをキャンセルできなさそうなので。
        enabled: addListDialog.visible && !addButton.enabled
        sequence: "Esc"
        onActivated: addListDialog.close()
    }

    SystemTool {
        id: systemTool
    }
    RecordOperator {
        id: recordOperator
        onFinished: (success) => {
                        if(success){
                            addListDialog.accept()
                        }
                    }
        onErrorOccured: (code, message) => addListDialog.errorOccured(account.uuid, code, message)
    }

    ColumnLayout {
        spacing: 5
        RowLayout {
            AvatarImage {
                Layout.preferredWidth: AdjustedValues.i24
                Layout.preferredHeight: AdjustedValues.i24
                source: account.avatar
            }
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

        Label {
            Layout.fillWidth: true
            Layout.topMargin: 5
            font.pointSize: AdjustedValues.f10
            text: qsTr("Avatar")
        }
        Rectangle {
            Layout.preferredWidth: AdjustedValues.i48
            Layout.preferredHeight: AdjustedValues.i48
            color: Material.frameColor
            Image {
                id: avatarImage
                anchors.fill: parent
                anchors.margins: 1
                source: "../images/edit.png"
                MouseArea {
                    anchors.fill: parent
                    onClicked: fileDialog.open()
                }
            }
        }

        Label {
            Layout.fillWidth: true
            Layout.topMargin: 5
            font.pointSize: AdjustedValues.f10
            text: qsTr("Name")
        }
        TextField  {
            id: nameText
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            enabled: !recordOperator.running
            selectByMouse: true
            font.pointSize: AdjustedValues.f10
            property int realTextLength: systemTool.countText(text)
        }
        Label {
            Layout.alignment: Qt.AlignRight
            font.pointSize: AdjustedValues.f8
            text: 64 - nameText.realTextLength
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
                onClicked: addListDialog.reject()
            }
            Item {
                Layout.fillWidth: true
                height: 1
            }
            Button {
                id: addButton
                Layout.alignment: Qt.AlignRight
                font.pointSize: AdjustedValues.f10
                enabled: nameText.text.length > 0 &&
                         nameText.realTextLength <= 64 &&
                         descriptionText.realTextLength <= 300 &&
                         !recordOperator.running
                text: addListDialog.editMode ? qsTr("Update") : qsTr("Add")
                onClicked: {
                    recordOperator.clear()
                    if(addListDialog.editMode){
                        recordOperator.updateList(addListDialog.listUri, avatarImage.source,
                                                  descriptionText.text, nameText.text)
                    }else{
                        if(avatarImage.status === Image.Ready && (avatarImage.source + "").indexOf("qrc:") !== 0){
                            recordOperator.setImages([avatarImage.source], [])
                        }
                        recordOperator.list(nameText.text, RecordOperator.Curation, descriptionText.text)
                    }
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
        onRejected: embedImage = ""
        onAccepted: {
            console.log("Selected=" + selectedX + "," + selectedY + "," + selectedWidth + "," + selectedHeight)
            avatarImage.source = systemTool.clipImage(embedImage,
                                                 selectedX, selectedY,
                                                 selectedWidth,selectedHeight)
        }
    }
}
