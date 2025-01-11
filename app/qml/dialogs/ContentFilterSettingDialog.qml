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
        }

        ComboBox {
            id: labelerDidComboBox
            Layout.fillWidth: true
            font.pointSize: AdjustedValues.f10
            textRole: "text"
            valueRole: "value"
            model: ListModel {}
            delegate: ItemDelegate {
                width: labelerDidComboBox.width
                height: implicitHeight * AdjustedValues.ratio
                font.pointSize: AdjustedValues.f10
                onClicked: labelerDidComboBox.currentIndex = model.index
                ColumnLayout {
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 3
                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: model.text
                    }
                    Label {
                        font.pointSize: AdjustedValues.f8
                        text: model.description.split("\n")[0]
                    }
                }
            }
            onCurrentValueChanged: {
                console.log("currentText=" + currentText + ", currentValue=" + currentValue)
                if(currentValue){
                    contentFilterSettingListModel.labelerDid = currentValue
                }
            }
        }

        CheckBox {
            id: enableAdultContentCheckbox
            Layout.bottomMargin: 0
            enabled: !contentFilterSettingListModel.running &&
                     contentFilterSettingDialog.ready &&
                     contentFilterSettingListModel.labelerHasAdultOnly
            font.pointSize: AdjustedValues.f10
            text: qsTr("Enable adult content")
        }

        ScrollView {
            id: settingScrollView
            Layout.bottomMargin: 10
            Layout.preferredWidth: 500 * AdjustedValues.ratio
            Layout.preferredHeight: 300 * AdjustedValues.ratio
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
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
                        labelerDidComboBox.currentIndex = applyButton.savingIndex
                    }
                }
                delegate: RowLayout {
                    id: listItemLayout
                    width: 500 * AdjustedValues.ratio - settingScrollView.ScrollBar.vertical.width
                    height: 5 + (labelLayout.height > selectButtonLayout.height ? labelLayout.height : selectButtonLayout.height)
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
                        id: labelLayout
                        spacing: 0
                        Label {
                            Layout.preferredWidth: 250 * AdjustedValues.ratio
                            Layout.maximumWidth: 480 * AdjustedValues.ratio - selectButtonLayout.width
                            font.pointSize: AdjustedValues.f10
                            text: model.title
                        }
                        Label {
                            Layout.preferredWidth: 250 * AdjustedValues.ratio
                            Layout.maximumWidth: 480 * AdjustedValues.ratio - selectButtonLayout.width
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
                        Layout.alignment: Qt.AlignTop
                        spacing: 3
                        Button {
                            height: AdjustedValues.b36
                            text: qsTr("Hide")
                            enabled: !model.isAdultImagery || (model.isAdultImagery && enableAdultContentCheckbox.checked)
                            highlighted: model.status === value
                            font.pointSize: AdjustedValues.f10
                            font.capitalization: Font.MixedCase
                            property int value: 0
                            onClicked: contentFilterSettingListModel.update(model.index,
                                                                            ContentFilterSettingListModel.StatusRole,
                                                                            value)
                        }
                        Button {
                            height: AdjustedValues.b36
                            text: model.level === 1 ? qsTr("Badge") : qsTr("Warn")
                            enabled: !model.isAdultImagery || (model.isAdultImagery && enableAdultContentCheckbox.checked)
                            highlighted: model.status === value
                            font.pointSize: AdjustedValues.f10
                            font.capitalization: Font.MixedCase
                            property int value: 1
                            onClicked: contentFilterSettingListModel.update(model.index,
                                                                            ContentFilterSettingListModel.StatusRole,
                                                                            value)
                        }
                        Button {
                            height: AdjustedValues.b36
                            text: qsTr("Show")
                            enabled: !model.isAdultImagery || (model.isAdultImagery && enableAdultContentCheckbox.checked)
                            highlighted: model.status === value
                            font.pointSize: AdjustedValues.f10
                            font.capitalization: Font.MixedCase
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
                id: applyButton
                enabled: !contentFilterSettingListModel.running && contentFilterSettingDialog.ready
                font.pointSize: AdjustedValues.f10
                highlighted: contentFilterSettingListModel.modified
                text: qsTr("Apply")
                property int savingIndex: 0
                onClicked: {
                    savingIndex = labelerDidComboBox.currentIndex
                    contentFilterSettingListModel.save()
                }
                BusyIndicator {
                    anchors.fill: parent
                    anchors.margins: 3
                    visible: contentFilterSettingListModel.running
                }
            }
        }
    }
}
