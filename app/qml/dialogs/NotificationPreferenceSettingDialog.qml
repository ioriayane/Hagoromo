import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.notificationpreferencelistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"

Dialog {
    id: notificationPreferenceSettingDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Notification Preferences")

    property bool ready: false
    property alias account: account
    Account {
        id: account
    }
    onOpened: {
        if(account.service.length === 0){
            return
        }
        notificationPreferenceListModel.load()
        notificationPreferenceSettingDialog.ready = true
    }
    onClosed: {
        notificationPreferenceSettingDialog.ready = false
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

        ScrollView {
            id: settingScrollView
            Layout.bottomMargin: 10
            Layout.preferredWidth: 500 * AdjustedValues.ratio
            Layout.preferredHeight: 400 * AdjustedValues.ratio
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            enabled: !notificationPreferenceListModel.running && notificationPreferenceSettingDialog.ready

            ListView {
                id: settingListView
                clip: true
                maximumFlickVelocity: AdjustedValues.maximumFlickVelocity
                model: NotificationPreferenceListModel {
                    id: notificationPreferenceListModel
                    account: account.uuid
                }
                
                section.property: "category"
                section.criteria: ViewSection.FullString
                section.labelPositioning: ViewSection.InlineLabels
                section.delegate: Rectangle {
                    id: sectionHeader
                    width: settingListView.width
                    height: sectionLabel.contentHeight * 1.2
                    color: Material.color(Material.Grey)
                    Label {
                        id: sectionLabel
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        font.pointSize: AdjustedValues.f10
                        font.bold: true
                        text: section
                    }
                }

                delegate: Rectangle {
                    id: listItemLayout
                    width: settingListView.width
                    height: model.enabled ? (contentLayout.height + 20 * AdjustedValues.ratio) : 0
                    visible: model.enabled
                    color: "transparent"

                    ColumnLayout {
                        id: contentLayout
                        spacing: 8 * AdjustedValues.ratio

                        Label {
                            Layout.fillWidth: true
                            Layout.topMargin: 8 * AdjustedValues.ratio
                            Layout.leftMargin: 10 * AdjustedValues.ratio
                            font.pointSize: AdjustedValues.f10
                            text: model.displayName
                            wrapMode: Text.Wrap
                        }

                        // ColumnLayout {
                        //     Layout.fillWidth: true
                        //     Layout.leftMargin: 10 * AdjustedValues.ratio
                        //     spacing: 3
                        //     Label {
                        //         Layout.fillWidth: true
                        //         font.pointSize: AdjustedValues.f10
                        //         text: model.displayName
                        //         wrapMode: Text.Wrap
                        //     }
                        //     Label {
                        //         Layout.fillWidth: true
                        //         Layout.leftMargin: 5 * AdjustedValues.ratio
                        //         wrapMode: Text.Wrap
                        //         font.pointSize: AdjustedValues.f8
                        //         color: Material.color(Material.Grey)
                        //         text: model.description
                        //         visible: model.description.length > 0
                        //     }
                        // }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: 20 * AdjustedValues.ratio
                            spacing: 15 * AdjustedValues.ratio

                            RowLayout {
                                visible: model.showList
                                spacing: 5 * AdjustedValues.ratio
                                Label {
                                    font.pointSize: AdjustedValues.f8
                                    text: qsTr("In-app notifications")
                                }
                                Switch {
                                    id: listSwitch
                                    checked: model.list
                                    onCheckedChanged: {
                                        if (checked !== model.list) {
                                            notificationPreferenceListModel.update(
                                                model.index,
                                                NotificationPreferenceListModel.ListRole,
                                                checked
                                            )
                                        }
                                    }
                                }
                            }

                            RowLayout {
                                spacing: 5 * AdjustedValues.ratio
                                Label {
                                    font.pointSize: AdjustedValues.f8
                                    text: qsTr("Push notifications")
                                }
                                Switch {
                                    id: pushSwitch
                                    checked: model.push
                                    onCheckedChanged: {
                                        if (checked !== model.push) {
                                            notificationPreferenceListModel.update(
                                                model.index,
                                                NotificationPreferenceListModel.PushRole,
                                                checked
                                            )
                                        }
                                    }
                                }
                            }
                        }

                        // Include settings - only show if the preference type supports it
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: 20 * AdjustedValues.ratio
                            spacing: 10 * AdjustedValues.ratio
                            visible: model.includeType !== undefined && model.includeType !== NotificationPreferenceListModel.NoInclude

                            Label {
                                font.pointSize: AdjustedValues.f8
                                text: qsTr("Include notifications from")
                            }

                            property var availableOptions: model.type !== undefined ? 
                                notificationPreferenceListModel.getAvailableIncludeOptions(model.type) : []
                            property int optionIndex: availableOptions.indexOf(model.include)
                            property int includeType: model.includeType

                            function updateInclude(newInclude) {
                                if (newInclude !== model.include) {
                                    notificationPreferenceListModel.update(
                                        model.index,
                                        NotificationPreferenceListModel.IncludeRole,
                                        newInclude
                                    )
                                }
                            }

                            ListModel {
                                id: includeOptionsFollowsModel
                                    ListElement { value: "all"; text: qsTr("Everyone") }
                                    ListElement { value: "follows"; text: qsTr("People I follow") }
                            }
                            ListModel {
                                id: includeOptionsAcceptedModel
                                    ListElement { value: "all"; text: qsTr("Everyone") }
                                    ListElement { value: "accepted"; text: qsTr("Accepted") }
                            }
                            ComboBoxEx {
                                id: includeComboBox
                                Layout.preferredWidth: 150 * AdjustedValues.ratio
                                Layout.preferredHeight: implicitHeight * AdjustedValues.ratio
                                font.pointSize: AdjustedValues.f8
                                model: parent.includeType === NotificationPreferenceListModel.FollowsInclude ? includeOptionsFollowsModel : includeOptionsAcceptedModel
                                onModelChanged: currentIndex = parent.optionIndex
                                onActivated: parent.updateInclude(currentValue)
                            }
                        }
                    }

                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 1
                        color: Material.color(Material.Grey, Material.Shade300)
                    }
                }
            }
        }

        RowLayout {
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Close")
                flat: true
                onClicked: notificationPreferenceSettingDialog.reject()
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                id: applyButton
                enabled: !notificationPreferenceListModel.running && notificationPreferenceSettingDialog.ready
                font.pointSize: AdjustedValues.f10
                highlighted: notificationPreferenceListModel.modified
                text: qsTr("Apply")
                onClicked: {
                    notificationPreferenceListModel.save()
                }
                BusyIndicator {
                    anchors.fill: parent
                    anchors.margins: 3
                    visible: notificationPreferenceListModel.running
                }
            }
        }
    }
}
