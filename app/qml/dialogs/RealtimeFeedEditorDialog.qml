import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.realtime.editselectorlistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"

Dialog {
    id: realtimeFeedEditorDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    closePolicy: Popup.CloseOnEscape
    title: qsTr("Realtime Feed Editor")

    property alias account: account
    Account {
        id: account
    }

    ColumnLayout {
        AccountLayout {
            Layout.fillWidth: true
            source: account.avatar
            handle: account.handle
        }
        RowLayout {
            spacing: 10
            states: [
                State {
                    when: editSelectorListView.currentItem && (
                              editSelectorListView.currentItem.type === "following" ||
                              editSelectorListView.currentItem.type === "followers" ||
                              editSelectorListView.currentItem.type === "list" ||
                              editSelectorListView.currentItem.type === "me"
                              )
                    PropertyChanges { target: selectorDetailLayout; visible: true }
                    PropertyChanges { target: dummyDetailLayout; visible: false }
                }
            ]

            ColumnLayout {
                Label {
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("Condition")
                }

                ScrollView {
                    Layout.preferredWidth: 350
                    Layout.preferredHeight: 300
                    ListView {
                        id: editSelectorListView
                        //ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                        //ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                        clip: true
                        model: EditSelectorListModel {
                            id: editSelectorListModel
                        }

                        header: SelectorDelegate {
                            id: headerSelector
                            indent: 0
                            index: -1
                            type: "new"
                            displayType: "New"
                            useAppendButton: true
                            onAppendChild: (row, type) => editSelectorListModel.appendChild(row, type)
                            visible: editSelectorListModel.count === 0
                            states: [
                                State {
                                    when: visible === false
                                    PropertyChanges {
                                        target: headerSelector
                                        height: 0
                                    }
                                }
                            ]
                        }

                        delegate: SelectorDelegate {
                            indent: model.indent
                            index: model.index
                            type: model.type
                            displayType: model.displayType
                            useAppendButton: model.canHave
                            highlighted:  model.index === editSelectorListView.currentIndex
                            onClicked: editSelectorListView.currentIndex = model.index
                            onAppendChild: (row, type) => editSelectorListModel.appendChild(row, type)
                            onRemove: (row) => editSelectorListModel.remove(row)
                        }
                    }
                }
            }
            Rectangle {
                Layout.preferredWidth: 1
                Layout.fillHeight: true
                color: Material.foreground
                opacity: 0.5
            }
            ColumnLayout {
                id: dummyDetailLayout
                Layout.preferredWidth: 200
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
            ColumnLayout {
                id: selectorDetailLayout
                Layout.preferredWidth: 200
                spacing: 0
                visible: false
                property int adjustedPadding: 6 * AdjustedValues.ratio
                Label {
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("Detail")
                }
                CheckBox {
                    topPadding: parent.adjustedPadding
                    bottomPadding: parent.adjustedPadding
                    text: qsTr("Has embeded")
                }
                CheckBox {
                    topPadding: parent.adjustedPadding
                    bottomPadding: parent.adjustedPadding
                    text: qsTr("Has quote")
                }
                // CheckBox {
                //     topPadding: parent.adjustedPadding
                //     bottomPadding: parent.adjustedPadding
                //     text: qsTr("Contains hashtag")
                // }
                // TextField {
                //     Layout.leftMargin: AdjustedValues.s20
                //     topPadding: parent.adjustedPadding
                //     // bottomPadding: parent.adjustedPadding
                // }
                // CheckBox {
                //     topPadding: parent.adjustedPadding
                //     bottomPadding: parent.adjustedPadding
                //     text: qsTr("Contains word")
                // }
                // TextField {
                //     Layout.leftMargin: AdjustedValues.s20
                //     topPadding: parent.adjustedPadding
                //     // bottomPadding: parent.adjustedPadding
                // }
                Item {
                    id: detailArea
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
        RowLayout {
            spacing: 10
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Cancel")
                flat: true
                onClicked: realtimeFeedEditorDialog.reject()
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                font.pointSize: AdjustedValues.f10
                // enabled: accountList.currentIndex >= 0
                text: qsTr("Submit")
                // onClicked: addColumnDialog.accept()
            }
        }

    }
}
