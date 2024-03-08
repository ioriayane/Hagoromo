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
    title: qsTr("Add/Remove muted words")

    property alias account: account
    Account {
        id: account
    }
    signal errorOccured(string account_uuid, string code, string message)

    onOpened: {
        if(account.service.length === 0){
            return
        }
        mutedWordListModel.load()
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
            TextField  {
                id: valueText
                Layout.preferredWidth: 400 * AdjustedValues.ratio
                enabled: !mutedWordListModel.running
                selectByMouse: true
                font.pointSize: AdjustedValues.f10
                placeholderText: qsTr("Enter a word or tag")
            }
            RadioButton {
                bottomPadding: AdjustedValues.s10
                font.pointSize: AdjustedValues.f10
                text: qsTr("Mute in text & tags")
            }
            RadioButton {
                topPadding: AdjustedValues.s10
                font.pointSize: AdjustedValues.f10
                text: qsTr("Mute in tags only")
            }
        }

        ScrollView {
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            Layout.preferredHeight: 300 * AdjustedValues.ratioHalf
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
                    onFinished: console.log("mutedWordListModel::finished=" + rowCount())
                }
                header: BusyIndicator {
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
                        Label {
                            Layout.alignment: Qt.AlignVCenter
                            font.pointSize: AdjustedValues.f10
                            text: model.value
                        }
                        // Label {
                        //     Layout.alignment: Qt.AlignVCenter
                        //     font.pointSize: AdjustedValues.f8
                        //     text: model.targets
                        // }
                    }
                    BusyIndicator {
                        anchors.right: parent.right
                        anchors.rightMargin: 20
                        anchors.verticalCenter: parent.verticalCenter
                        width: AdjustedValues.i18
                        height: AdjustedValues.i18
                        visible: mutedWordListModel.running
                    }
                    onClicked: {
                        console.log("Remove from list(" + model.index + "):" + model.value + " - " + model.targets)
                        // listsListModel.addRemoveFromList(model.index, addToListDialog.targetDid)
                    }
                }
            }
        }
        RowLayout {
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Close")
                flat: true
                onClicked: addMutedWordDialog.reject()
            }
            Item {
                Layout.fillWidth: true
            }
        }
    }
}
