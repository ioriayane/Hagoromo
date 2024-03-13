import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.mutedwordlistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"

Dialog {
    id: addMutedWordDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    closePolicy: Popup.NoAutoClose
    title: qsTr("Update muted words")

    property string initialValue: ""
    property alias account: account
    Account {
        id: account
    }
    signal errorOccured(string account_uuid, string code, string message)

    onOpened: {
        if(initialValue.length > 0){
            valueText.text = initialValue
        }
        initialValue = ""
        muteTextAndTagRadioButton.checked = true
        if(account.service.length === 0){
            return
        }
        mutedWordListModel.modified = false
        mutedWordListModel.load()
    }
    onClosed: {
        valueText.clear()
        mutedWordListModel.clear()
    }

    Shortcut {  // Close
        enabled: !mutedWordListModel.modified
        sequence: "Esc"
        onActivated: postDialog.close()
    }

    ColumnLayout {
        spacing: 10
        RowLayout {
            AvatarImage {
                Layout.preferredWidth: AdjustedValues.i24
                Layout.preferredHeight: AdjustedValues.i24
                source: account.avatar
            }
            Label {
                Layout.fillWidth: true
                font.pointSize: AdjustedValues.f10
                text: account.handle
                elide: Text.ElideRight
            }
            IconButton {
                Layout.preferredWidth: AdjustedValues.b30
                Layout.preferredHeight: AdjustedValues.b24
                iconSource: "../images/refresh.png"
                iconSize: AdjustedValues.i16
                onClicked: mutedWordListModel.load()
            }
        }
        ColumnLayout {
            spacing: 0
            RowLayout {
                TextField  {
                    id: valueText
                    Layout.fillWidth: true
                    enabled: !mutedWordListModel.running
                    selectByMouse: true
                    font.pointSize: AdjustedValues.f10
                    placeholderText: qsTr("Enter a word or tag")
                }
                Button {
                    id: addButton
                    enabled: valueText.text.length > 0
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("Add/Update")
                    onClicked: {
                        var value = valueText.text.trim()
                        if(value.charAt(0) === "#"){
                            value = value.substring(1)
                        }
                        var row = mutedWordListModel.append(value, muteTagOnlyRadioButton.checked)
                        valueText.text = ""
                        mutedWordListView.positionViewAtIndex(row, ListView.End)
                    }
                }
            }
            RowLayout {
                RadioButton {
                    id: muteTextAndTagRadioButton
                    //bottomPadding: AdjustedValues.s10
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("Mute in text & tags")
                }
                RadioButton {
                    id: muteTagOnlyRadioButton
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("Mute in tags only")
                }
            }
        }

        ScrollView {
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            Layout.preferredHeight: 300 * AdjustedValues.ratioHalf
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            clip: true

            ListView {
                id: mutedWordListView
                anchors.fill: parent
                anchors.rightMargin: parent.ScrollBar.vertical.width
                maximumFlickVelocity: AdjustedValues.maximumFlickVelocity

                model: MutedWordListModel {
                    id: mutedWordListModel
                    service: addMutedWordDialog.account.service
                    handle: addMutedWordDialog.account.handle
                    accessJwt: addMutedWordDialog.account.accessJwt
                    onFinished: modified = false
                }
                footer: BusyIndicator {
                    width: mutedWordListView.width
                    height: AdjustedValues.i18
                    visible: mutedWordListModel.running
                }
                delegate: ItemDelegate {
                    width: mutedWordListView.width
                    height: implicitHeight * AdjustedValues.ratio
                    RowLayout {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        property var targets: model.targets
                        Label {
                            Layout.preferredWidth: 270 * AdjustedValues.ratio
                            Layout.alignment: Qt.AlignVCenter
                            font.pointSize: AdjustedValues.f10
                            text: model.value
                            elide: Text.ElideRight
                        }
                        Item {
                            Layout.preferredHeight: 1
                            Layout.fillWidth: true
                        }
                        Repeater {
                            model: parent.targets
                            Label {
                                Layout.rightMargin: 3
                                Layout.alignment: Qt.AlignVCenter
                                font.pointSize: AdjustedValues.f8
                                text: modelData
                                Rectangle {
                                    anchors.fill: parent
                                    anchors.leftMargin: -2
                                    anchors.rightMargin: -2
                                    z: -1
                                    radius: 3
                                    color: Material.color(Material.BlueGrey)
                                }
                            }
                        }
                        IconButton {
                            Layout.preferredHeight: AdjustedValues.b26
                            iconSource: "../images/delete.png"
                            onClicked: mutedWordListModel.remove(model.index)
                        }
                    }
                    onClicked: {
                        if(model.targets.includes("text")){
                            muteTextAndTagRadioButton.checked = true
                        }else{
                            muteTagOnlyRadioButton.checked = true
                        }
                        valueText.text = model.value
                    }
                }
            }
        }
        RowLayout {
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Close")
                flat: true
                onClicked: {
                    if(mutedWordListModel.modified){
                        msgDialog.show("normal", "", qsTr("Close without saving?"))
                    }else{
                        addMutedWordDialog.reject()
                    }
                }
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                font.pointSize: AdjustedValues.f10
                enabled: !mutedWordListModel.running
                text: qsTr("Save") + (mutedWordListModel.modified ? "(*)" : "")
                onClicked: mutedWordListModel.save()
            }
        }
    }

    MessageDialog {
        id: msgDialog
        useCancel: true
        onAccepted: addMutedWordDialog.close()
    }
}
