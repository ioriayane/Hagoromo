import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.feedgeneratorlistmodel 1.0

import "../controls"
import "../parts"

Dialog {
    id: discoverFeedsDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Discover Feeds")

    property alias account: account
    Account {
        id: account
    }
    property string selectedName: ""
    property string selectedUri: ""

    onOpened: {
        if(account.service.length === 0){
            return
        }

        feedGeneratorListModel.setAccount(account.service, account.did, account.handle,
                                          account.email, account.accessJwt, account.refreshJwt)
        feedGeneratorListModel.getLatest()

        generatorListView.currentIndex = -1
    }

    ColumnLayout {
        spacing: 10
        RowLayout {
            AvatarImage {
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                source: account.avatar
            }
            Label {
                text: account.handle
                elide: Text.ElideRight
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
        }

        ScrollView {
            id: generatorScrollView
            Layout.preferredWidth: 400
            Layout.preferredHeight: 350
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            ListView {
                id: generatorListView
                clip: true
                model: FeedGeneratorListModel {
                    id: feedGeneratorListModel
                }
                footer: BusyIndicator {
                    width: generatorListView.width
                    height: 32
                    visible: feedGeneratorListModel.running
                }
                delegate: ItemDelegate {
                    width: generatorListView.width
                    height: contentLayout.height + 10
                    highlighted: ListView.isCurrentItem
                    onClicked: {
                        generatorListView.currentIndex = model.index

                        discoverFeedsDialog.selectedName = model.displayName
                        discoverFeedsDialog.selectedUri = model.uri
                    }
                    states: [
                        State {
                            when: feedGeneratorListModel.running
                            PropertyChanges {
                                target: saveButton
                                iconSource: ""
                                enabled: false
                            }
                        },
                        State {
                            when: model.saving === true
                            PropertyChanges {
                                target: saveButton
                                iconSource: "../images/bookmark_add.png"
                                foreground: Material.color(Material.Pink)
                                onClicked: feedGeneratorListModel.removeGenerator(model.uri)
                            }
                        },
                        State {
                            when: model.saving === false
                            PropertyChanges {
                                target: saveButton
                                iconSource: "../images/bookmark_add.png"
                                foreground: Material.color(Material.Grey)
                                onClicked: feedGeneratorListModel.saveGenerator(model.uri)
                            }
                        }
                    ]
                    ColumnLayout {
                        id: contentLayout
                        GridLayout {
                            Layout.topMargin: 10
                            Layout.leftMargin: 10
                            Layout.rightMargin: 20
                            columns: 2
                            AvatarImage {
                                Layout.preferredWidth: 24
                                Layout.preferredHeight: 24
                                Layout.rowSpan: 2
                                source: model.avatar
                                altSource: "../images/account_icon.png"
                            }
                            Label {
                                Layout.fillWidth: true
                                font.pointSize: 10
                                text: model.displayName
                            }
                            Label {
                                color: Material.color(Material.Grey)
                                font.pointSize: 8
                                text: "by @" + model.creatorDisplayName
                            }
                        }
                        Label {
                            Layout.leftMargin: 10
                            Layout.rightMargin: 20
                            Layout.preferredWidth: generatorListView.width - 30
                            wrapMode: Text.WrapAnywhere
                            font.pointSize: 10
                            text: model.description
                        }
                        RowLayout {
                            Layout.leftMargin: 10
                            Layout.rightMargin: 10
                            Image {
                                Layout.preferredWidth: 16
                                Layout.preferredHeight: 16
                                source: "../images/like.png"
                                layer.enabled: true
                                layer.effect: ColorOverlay {
                                    color: Material.color(Material.Pink)
                                }
                            }
                            Label {
                                Layout.fillWidth: true
                                font.pointSize: 8
                                text: model.likeCount
                            }
                        }
                    }
                    IconButton {
                        id: saveButton
                        anchors.top: parent.top
                        anchors.topMargin: 10
                        anchors.right: parent.right
                        anchors.rightMargin: 20
                        width: 36
                        height: 26
                        display: AbstractButton.IconOnly
                        iconSize: 18
                        BusyIndicator {
                            anchors.fill: parent
                            visible: feedGeneratorListModel.running
                        }
                    }
                }
            }
        }
        RowLayout {
            Button {
                text: qsTr("Cancel")
                flat: true
                onClicked: discoverFeedsDialog.reject()
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                enabled: generatorListView.currentIndex >= 0 && discoverFeedsDialog.selectedUri.length > 0
                text: qsTr("Add")
                onClicked: discoverFeedsDialog.accept()
            }
        }
    }
}
