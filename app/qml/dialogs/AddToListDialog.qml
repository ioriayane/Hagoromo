import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.listslistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"

Dialog {
    id: addToListDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Add/Remove from lists")

    property string targetDid: ""
    property alias account: account
    Account {
        id: account
    }
    signal errorOccured(string account_uuid, string code, string message)

    onOpened: {
        if(account.service.length === 0){
            return
        }

        listsListView.model.setAccount(account.service, account.did, account.handle,
                                       account.email, account.accessJwt, account.refreshJwt)
        listsListView.model.getLatest()
    }
    onClosed: {
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
        ScrollView {
            id: listsListScroll
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            Layout.preferredHeight: 350 * AdjustedValues.ratioHalf
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            clip: true

            ListView {
                id: listsListView
                anchors.fill: parent
                anchors.rightMargin: parent.ScrollBar.vertical.width

                model: ListsListModel {
                    actor: account.did
                    visibilityType: ListsListModel.VisibilityTypeCuration
                }
                footer:  ItemDelegate {
                    width: listsListView.width - listsListScroll.ScrollBar.vertical.width
                    height: implicitHeight * AdjustedValues.ratio
                    BusyIndicator {
                        id: busyIndicator
                        anchors.centerIn: parent
                        height: AdjustedValues.i32
                        visible: listsListView.model ? listsListView.model.running : false
                    }
                    RowLayout {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.margins: 10
                        visible: !busyIndicator.visible
                        AvatarImage {
                            Layout.preferredWidth: AdjustedValues.i24
                            Layout.preferredHeight: AdjustedValues.i24
                            altSource: "../images/list.png"
                        }
                        Label {
                            font.pointSize: AdjustedValues.f10
                            text: qsTr("Add list")
                        }
                    }
                    onClicked: {
                        if(busyIndicator.visible){
                            return
                        }
                        //addColumnDialog.openDiscoverFeeds(accountList.currentIndex)
                    }
                }
                delegate: ItemDelegate {
                    width: listsListView.width - listsListScroll.ScrollBar.vertical.width
                    height: implicitHeight * AdjustedValues.ratio
                    states: [
                        State {
                            when: model.searchStatus === ListsListModel.SearchStatusTypeRunning
                            PropertyChanges { target: saveButton; iconSource: ""; enabled: false }
                        },
                        State {
                            when: model.searchStatus === ListsListModel.SearchStatusTypeUnknown
                            PropertyChanges { target: saveButton; iconSource: ""; enabled: false }
                        },
                        State {
                            when: model.searchStatus === ListsListModel.SearchStatusTypeContains
                            PropertyChanges {
                                target: saveButton
                                iconSource: "../images/bookmark_add.png"
                                foreground: Material.color(Material.Pink)
                                //                            onClicked: generatorScrollView.requestRemoveGenerator(model.uri)
                            }
                        },
                        State {
                            when: model.searchStatus === ListsListModel.SearchStatusTypeNotContains
                            PropertyChanges {
                                target: saveButton
                                iconSource: "../images/bookmark_add.png"
                                foreground: Material.color(Material.Grey)
                                //                            onClicked: generatorScrollView.requestSaveGenerator(model.uri)
                            }
                        }
                    ]
                    RowLayout {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        AvatarImage {
                            Layout.preferredWidth: AdjustedValues.i24
                            Layout.preferredHeight: AdjustedValues.i24
                            Layout.alignment: Qt.AlignVCenter
                            source: model.avatar
                            altSource: "../images/list.png"
                        }
                        Label {
                            Layout.alignment: Qt.AlignVCenter
                            font.pointSize: AdjustedValues.f10
                            text: model.name
                        }
                    }
                    IconButton {
                        id: saveButton
                        anchors.right: parent.right
                        anchors.rightMargin: 20
                        anchors.verticalCenter: parent.verticalCenter
                        width: AdjustedValues.b36
                        height: AdjustedValues.b26
                        display: AbstractButton.IconOnly
                        iconSize: AdjustedValues.i18
                        flat: true
                        BusyIndicator {
                            anchors.fill: parent
                            visible: model.searchStatus === ListsListModel.SearchStatusTypeRunning
                        }
                    }
                    onClicked: {
                        console.log("Add or remove from list:" + model.uri + " <- " + addToListDialog.targetDid)
                    }
                }
            }
        }
        RowLayout {
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Close")
                flat: true
                onClicked: addToListDialog.reject()
            }
            Item {
                Layout.fillWidth: true
            }
        }
    }
}
