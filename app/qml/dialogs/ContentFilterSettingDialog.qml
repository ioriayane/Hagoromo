import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.contentfiltersettinglistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"

Dialog {
    id: contentFilterSettingDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Content Filtering")

    property bool ready: false
    property alias account: account
    Account {
        id: account
    }
    onOpened: {
        if(account.service.length === 0){
            return
        }
        contentFilterSettingListModel.load()
        contentFilterSettingDialog.ready = true
    }
    onClosed: {
        contentFilterSettingDialog.ready = false

    }

    ColumnLayout {
        spacing: 0
        RowLayout {
            Layout.bottomMargin: 10
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
            BusyIndicator {
                Layout.preferredWidth: AdjustedValues.i24
                Layout.preferredHeight: AdjustedValues.i24
                visible: contentFilterSettingListModel.running
            }
        }

        ComboBoxEx {
            id: labelerDidComboBox
            Layout.fillWidth: true
            model: ListModel {}
            delegate: ItemDelegate {
                width: parent.width
                height: implicitHeight * AdjustedValues.ratio
                font.pointSize: AdjustedValues.f10
                onClicked: {
                    labelerDidComboBox.currentIndex = model.index
                    contentFilterSettingListModel.selectableLabelerDescription(model.value)
                }
                ColumnLayout {
                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: model.text
                    }
                    Label {
                        font.pointSize: AdjustedValues.f8
                        text: model.description
                    }
                }
            }
            onCurrentValueChanged: {
                console.log("currentText=" + currentText + ", currentValue=" + currentValue)
                contentFilterSettingListModel.labelerDid = currentValue
            }
        }

        CheckBox {
            id: enableAdultContentCheckbox
            Layout.bottomMargin: 0
            enabled: !contentFilterSettingListModel.running && contentFilterSettingDialog.ready
            font.pointSize: AdjustedValues.f10
            text: qsTr("Enable adult content")
        }

        ScrollView {
            id: settingScrollView
            Layout.bottomMargin: 10
            Layout.preferredWidth: 450 * AdjustedValues.ratio
            Layout.preferredHeight: 350 * AdjustedValues.ratio
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            enabled: !contentFilterSettingListModel.running && contentFilterSettingDialog.ready

            ListView {
                id: settingListView
                clip: true
                maximumFlickVelocity: AdjustedValues.maximumFlickVelocity
                model: ContentFilterSettingListModel {
                    id: contentFilterSettingListModel
                    enableAdultContent: enableAdultContentCheckbox.checked
                    service: account.service
                    handle: account.handle
                    accessJwt: account.accessJwt
                    onFinished: {
                        enableAdultContentCheckbox.checked = enableAdultContent
                        labelerDidComboBox.model.clear()
                        var did = ""
                        for(var i=0; i<contentFilterSettingListModel.selectableLabelerDids.length; i++){
                            did = contentFilterSettingListModel.selectableLabelerDids[i]
                            labelerDidComboBox.model.append({
                                                                text: contentFilterSettingListModel.selectableLabelerName(did),
                                                                description: contentFilterSettingListModel.selectableLabelerDescription(did),
                                                                value: did
                                                            })
                        }
                        labelerDidComboBox.currentIndex = 0
                    }
                }
                delegate: RowLayout {
                    id: listItemLayout
                    width: 450 * AdjustedValues.ratio
                    spacing: 0
                    clip: true
                    states: [
                        State {
                            when: !model.configurable
                            PropertyChanges {
                                target: listItemLayout
                                visible: false
                                height: 0
                            }
                        }
                    ]
                    ColumnLayout {
                        Layout.topMargin: 10
                        //                        Layout.bottomMargin: 5
                        Label {
                            font.pointSize: AdjustedValues.f10
                            text: model.title
                        }
                        Label {
                            Layout.preferredWidth: 250 * AdjustedValues.ratio
                            Layout.maximumWidth: 450 * AdjustedValues.ratio - selectButtonLayout.width - 10
                            wrapMode: Text.Wrap
                            font.pointSize: AdjustedValues.f8
                            color: Material.color(Material.Grey)
                            text: model.description
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                    }
                    RowLayout {
                        id: selectButtonLayout
                        spacing: 0
                        IconButton {
                            Layout.preferredWidth: AdjustedValues.b55
                            Layout.preferredHeight: AdjustedValues.b36
                            iconText: qsTr("Hide")
                            enabled: !model.isAdultImagery || (model.isAdultImagery && enableAdultContentCheckbox.checked)
                            highlighted: model.status === value
                            property int value: 0
                            onClicked: contentFilterSettingListModel.update(model.index,
                                                                            ContentFilterSettingListModel.StatusRole,
                                                                            value)
                        }
                        IconButton {
                            Layout.preferredWidth: AdjustedValues.b55
                            Layout.preferredHeight: AdjustedValues.b36
                            iconText: qsTr("Warn")
                            enabled: !model.isAdultImagery || (model.isAdultImagery && enableAdultContentCheckbox.checked)
                            highlighted: model.status === value
                            property int value: 1
                            onClicked: contentFilterSettingListModel.update(model.index,
                                                                            ContentFilterSettingListModel.StatusRole,
                                                                            value)
                        }
                        IconButton {
                            Layout.preferredWidth: AdjustedValues.b55
                            Layout.preferredHeight: AdjustedValues.b36
                            iconText: qsTr("Show")
                            enabled: !model.isAdultImagery || (model.isAdultImagery && enableAdultContentCheckbox.checked)
                            highlighted: model.status === value
                            property int value: 2
                            onClicked: contentFilterSettingListModel.update(model.index,
                                                                            ContentFilterSettingListModel.StatusRole,
                                                                            value)
                        }
                    }
                }
            }
        }

        RowLayout {
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Cancel")
                flat: true
                onClicked: contentFilterSettingDialog.reject()
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                enabled: !contentFilterSettingListModel.running && contentFilterSettingDialog.ready
                font.pointSize: AdjustedValues.f10
                text: qsTr("Accept")
                onClicked: {
                    contentFilterSettingListModel.save()
                }
            }
        }
    }
}
