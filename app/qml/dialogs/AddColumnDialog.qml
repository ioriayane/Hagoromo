import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.accountlistmodel 1.0
import tech.relog.hagoromo.feedtypelistmodel 1.0

import "../parts"

Dialog {
    id: addColumnDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Add column")

    property alias accountModel: accountList.model

    property alias selectedAccountIndex: accountList.currentIndex
    property alias selectedTypeIndex: typeList.currentIndex

    onOpened: {
        feedTypeListModel.setAccount(accountModel.item(0, AccountListModel.ServiceRole),
                                     accountModel.item(0, AccountListModel.DidRole),
                                     accountModel.item(0, AccountListModel.HandleRole),
                                     accountModel.item(0, AccountListModel.EmailRole),
                                     accountModel.item(0, AccountListModel.AccessJwtRole),
                                     accountModel.item(0, AccountListModel.RefreshJwtRole))
        feedTypeListModel.getLatest()
    }

    ColumnLayout {
        RowLayout {
            spacing: 0

            ColumnLayout {
                Label {
                    text: qsTr("Account")
                }
                ScrollView {
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: 300
                    //ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    clip: true

                    ListView {
                        id: accountList
                        delegate: ItemDelegate {
                            width: accountList.width
                            highlighted: ListView.isCurrentItem
                            onClicked: accountList.currentIndex = model.index

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 5
                                AvatarImage {
                                    Layout.preferredWidth: 24
                                    Layout.preferredHeight: 24
                                    source: model.avatar
                                }
                                Label {
                                    text: model.handle
                                    elide: Text.ElideRight
                                }
                                Item {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 1
                                }
                            }
                        }
                    }
                }
            }
            Image {
                source: "../images/arrow_forward.png"
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                layer.enabled: true
                layer.effect: ColorOverlay {
                    color: Material.color(Material.Grey)
                }
            }
            ColumnLayout {
                Label {
                    text: qsTr("Column type")
                }
                ScrollView {
                    Layout.preferredWidth: 300
                    Layout.preferredHeight: 300
                    //ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    clip: true

                    ListView {
                        id: typeList
                        model: FeedTypeListModel {
                            id: feedTypeListModel
                        }
                        footer: BusyIndicator {
                            width: typeList.width
                            height: 24
                            visible: feedTypeListModel.running
                        }

                        delegate: ItemDelegate {
                            width: typeList.width
                            highlighted: ListView.isCurrentItem
                            onClicked: typeList.currentIndex = model.index
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 5
                                AvatarImage {
                                    Layout.preferredWidth: 24
                                    Layout.preferredHeight: 24
                                    source: model.avatar
                                    altSource: {
                                        if(model.feedType === 0){
                                            return "../images/home.png"
                                        }else if(model.feedType === 1){
                                            return "../images/notification.png"
                                        }else{
                                            return "../images/account_icon.png"
                                        }
                                    }
                                }
                                Label {
                                    font.pointSize: 10
                                    text: model.displayName
                                }
                                Label {
                                    color: Material.color(Material.Grey)
                                    font.pointSize: 10
                                    text: model.creatorDisplayName.length > 0 ? "by @" + model.creatorDisplayName : model.creatorDisplayName
                                }
                                Item {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 1
                                }
                            }
                        }
                    }
                }
            }
        }
        RowLayout {
            Button {
                text: qsTr("Cancel")
                flat: true
                onClicked: addColumnDialog.reject()
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                enabled: accountList.currentIndex >= 0 && typeList.currentIndex >= 0
                text: qsTr("Add")
                onClicked: addColumnDialog.accept()
            }
        }
    }
}
