import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

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
    property string pronouns: ""
    property string website: ""

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
        avatarImage.source = editProfileDialog.avatar
        bannerImage.source = editProfileDialog.banner
        displayNameText.text = editProfileDialog.displayName
        descriptionText.text = editProfileDialog.description
        pronounsText.text = editProfileDialog.pronouns
        websiteText.text = editProfileDialog.website
    }
    onClosed: {
        websiteText.clear()
        pronounsText.clear()
        displayNameText.clear()
        descriptionText.clear()
        avatarImage.source = ""
        bannerImage.source = ""
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
        onErrorOccured: (code, message) => editProfileDialog.errorOccured(account.uuid, code, message)
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

        Rectangle {
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            Layout.preferredHeight: 80
            color: Material.frameColor
            ImageWithIndicator {
                id: bannerImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                Rectangle {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: 2
                    width: AdjustedValues.i24
                    height: AdjustedValues.i24
                    radius: width / 2
                    border.color: "black"
                    border.width: 1
                    Image {
                        anchors.fill: parent
                        anchors.margins: 5
                        source: "../images/edit.png"
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: (mouse) => {
                                   imageClipDialog.target = "banner"
                                   imageClipDialog.squareMode = false
                                   fileDialog.open()
                               }
                }
            }
        }
        Rectangle {
            Layout.preferredWidth: AdjustedValues.i48
            Layout.preferredHeight: AdjustedValues.i48
            color: Material.frameColor
            radius: width / 2
            AvatarImage {
                id: avatarImage
                anchors.fill: parent
                onClicked: (mouse) => {
                               imageClipDialog.target = "avatar"
                               imageClipDialog.squareMode = true
                               fileDialog.open()
                           }
                Rectangle {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: 2
                    width: AdjustedValues.i18
                    height: AdjustedValues.i18
                    radius: width / 2
                    border.color: "black"
                    border.width: 1
                    Image {
                        anchors.fill: parent
                        anchors.margins: 3
                        source: "../images/edit.png"
                    }
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
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            enabled: !recordOperator.running
            selectByMouse: true
            font.pointSize: AdjustedValues.f10
            property int realTextLength: systemTool.countText(text)
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
            text: 256 - descriptionText.realTextLength
        }

        Label {
            Layout.fillWidth: true
            Layout.topMargin: 5
            font.pointSize: AdjustedValues.f10
            text: qsTr("Pronouns")
        }
        TextField  {
            id: pronounsText
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            enabled: !recordOperator.running
            selectByMouse: true
            font.pointSize: AdjustedValues.f10
            property int realTextLength: systemTool.countText(text)
        }

        Label {
            Layout.fillWidth: true
            Layout.topMargin: 5
            font.pointSize: AdjustedValues.f10
            text: qsTr("Website")
        }
        TextField  {
            id: websiteText
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            enabled: !recordOperator.running
            selectByMouse: true
            font.pointSize: AdjustedValues.f10
            property int realTextLength: systemTool.countText(text)
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
                enabled: displayNameText.realTextLength <= 64 && descriptionText.realTextLength <= 256 && !recordOperator.running
                text: qsTr("Update")
                onClicked: {
                    recordOperator.clear()
                    recordOperator.updateProfile(avatarImage.source, bannerImage.source,
                                                 descriptionText.text, displayNameText.text,
                                                 pronounsText.text, websiteText.text)
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
            var clipped = systemTool.clipImage(embedImage,
                                               selectedX, selectedY,
                                               selectedWidth, selectedHeight)
            if(target === "avatar"){
                avatarImage.source = clipped
            }else if(target === "banner"){
                bannerImage.source = clipped
            }
        }
    }
}
