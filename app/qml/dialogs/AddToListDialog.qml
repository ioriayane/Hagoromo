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
    signal requestAddList()

    onOpened: {
        if(account.service.length === 0){
            return
        }
        addListDialog.account.uuid = account.uuid
        addListDialog.account.service = account.service
        addListDialog.account.did = account.did
        addListDialog.account.handle = account.handle
        addListDialog.account.accessJwt = account.accessJwt
        addListDialog.account.refreshJwt = account.refreshJwt
        addListDialog.account.avatar = account.avatar

        listsListModel.clear()
        listsListModel.setAccount(account.service, account.did, account.handle,
                                       account.email, account.accessJwt, account.refreshJwt)
        listsListModel.getLatest()
    }
    onClosed: {
        listsListModel.clear()
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
                onClicked: {
                    listsListModel.clearListItemCache()
                    listsListModel.getLatest()
                }
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
                    id: listsListModel
                    actor: account.did
                    visibilityType: ListsListModel.VisibilityTypeCuration
                    searchTarget: addToListDialog.targetDid
                }
                footer:  ItemDelegate {
                    width: listsListView.width// - listsListScroll.ScrollBar.vertical.width
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
                            altSource: "../images/add.png"
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
                        addListDialog.open()
                    }
                }
                delegate: ItemDelegate {
                    width: listsListView.width// - listsListScroll.ScrollBar.vertical.width
                    height: implicitHeight * AdjustedValues.ratio
                    states: [
                        State {
                            when: model.searchStatus === ListsListModel.SearchStatusTypeRunning
                            PropertyChanges { target: saveButton; source: ""; enabled: false }
                        },
                        State {
                            when: model.searchStatus === ListsListModel.SearchStatusTypeUnknown
                            PropertyChanges { target: saveButton; source: ""; enabled: false }
                        },
                        State {
                            when: model.searchStatus === ListsListModel.SearchStatusTypeContains
                            PropertyChanges {
                                target: saveButton
                                source: "../images/bookmark_add.png"
                                foreground: Material.accentColor
                                //                            onClicked: generatorScrollView.requestRemoveGenerator(model.uri)
                            }
                        },
                        State {
                            when: model.searchStatus === ListsListModel.SearchStatusTypeNotContains
                            PropertyChanges {
                                target: saveButton
                                source: "../images/bookmark_add.png"
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
                    Image {
                        id: saveButton
                        anchors.right: parent.right
                        anchors.rightMargin: 20
                        anchors.verticalCenter: parent.verticalCenter
                        width: AdjustedValues.i18
                        height: AdjustedValues.i18
                        layer.enabled: true
                        layer.effect: ColorOverlay {
                            color: saveButton.foreground
                        }
                        property color foreground: Material.foreground
                        BusyIndicator {
                            anchors.fill: parent
                            visible: model.searchStatus === ListsListModel.SearchStatusTypeRunning
                        }
                    }
                    onClicked: {
                        console.log("Add or remove from list(" + model.index + "):" + model.uri + " <- " + addToListDialog.targetDid)
                        listsListModel.addRemoveFromList(model.index, addToListDialog.targetDid)
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

    AddListDialog {
        id: addListDialog
        onAccepted: {
            listsListModel.clear()
            listsListModel.getLatest()
        }
    }
}
