import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.feedgeneratorlistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
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
    onClosed: {
        searchText.text = ""
    }

    Shortcut {  // Search
        enabled: searchButton.enabled && searchText.focus
        sequence: "Ctrl+Return"
        onActivated: searchButton.clicked()
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
                font.pointSize: AdjustedValues.f10
                text: account.handle
                elide: Text.ElideRight
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
        }

        RowLayout {
            spacing: AdjustedValues.s5
            TextField  {
                id: searchText
                Layout.fillWidth: true
                selectByMouse: true
                font.pointSize: AdjustedValues.f10
            }
            Button {
                id: searchButton
                enabled: searchText.text.length > 0 && !feedGeneratorListModel.running
                font.pointSize: AdjustedValues.f10
                text: qsTr("Search")
                onClicked: {
                    discoverFeedsDialog.selectedName = ""
                    discoverFeedsDialog.selectedUri = ""
                    feedGeneratorListModel.getLatest()
                }
                BusyIndicator {
                    anchors.fill: parent
                    anchors.margins: 3
                    visible: feedGeneratorListModel.running
                }
            }
        }

        ScrollView {
            id: generatorScrollView
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            Layout.preferredHeight: 350 * AdjustedValues.ratioHalf
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            ListView {
                id: generatorListView
                clip: true
                onMovementEnded: {
                    if(atYEnd){
                        feedGeneratorListModel.getNext()
                    }
                }
                model: FeedGeneratorListModel {
                    id: feedGeneratorListModel
                    query: searchText.text
                }
                footer: BusyIndicator {
                    width: generatorListView.width - generatorScrollView.ScrollBar.vertical.width
                    height: AdjustedValues.h32
                    visible: feedGeneratorListModel.running
                }
                delegate: ItemDelegate {
                    width: generatorListView.width - generatorScrollView.ScrollBar.vertical.width
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
                                Layout.preferredWidth: AdjustedValues.i24
                                Layout.preferredHeight: AdjustedValues.i24
                                Layout.rowSpan: 2
                                source: model.avatar
                                altSource: "../images/account_icon.png"
                            }
                            Label {
                                Layout.fillWidth: true
                                font.pointSize: AdjustedValues.f10
                                text: model.displayName
                            }
                            Label {
                                color: Material.color(Material.Grey)
                                font.pointSize: AdjustedValues.f8
                                text: "by @" + model.creatorDisplayName
                            }
                        }
                        Label {
                            Layout.leftMargin: 10
                            Layout.rightMargin: 20
                            Layout.preferredWidth: generatorListView.width - 30
                            wrapMode: Text.WrapAnywhere
                            font.pointSize: AdjustedValues.f10
                            text: model.description
                        }
                        RowLayout {
                            Layout.leftMargin: 10
                            Layout.rightMargin: 10
                            Image {
                                Layout.preferredWidth: AdjustedValues.i16
                                Layout.preferredHeight: AdjustedValues.i16
                                source: "../images/like.png"
                                layer.enabled: true
                                layer.effect: ColorOverlay {
                                    color: Material.color(Material.Pink)
                                }
                            }
                            Label {
                                Layout.fillWidth: true
                                font.pointSize: AdjustedValues.f8
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
                        width: AdjustedValues.b36
                        height: AdjustedValues.b26
                        display: AbstractButton.IconOnly
                        iconSize: AdjustedValues.i18
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
                font.pointSize: AdjustedValues.f10
                text: qsTr("Cancel")
                flat: true
                onClicked: discoverFeedsDialog.reject()
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                id: addButton
                enabled: generatorListView.currentIndex >= 0 && discoverFeedsDialog.selectedUri.length > 0
                font.pointSize: AdjustedValues.f10
                text: qsTr("Add")
                onClicked: {
                    discoverFeedsDialog.accept()
                }
            }
        }
    }
}
