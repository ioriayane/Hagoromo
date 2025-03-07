import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.realtime.editselectorlistmodel 1.0
import tech.relog.hagoromo.listslistmodel 1.0
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

    function setupAndOpen(display_name, condition){
        listComboBox.currentIndex = -1
        listsListModel.clear()
        listsListModel.setAccount(account.uuid)
        listsListModel.getLatest()

        editSelectorListView.currentIndex = -1
        ruleNameTextField.text = display_name
        editSelectorListModel.selectorJson = condition
        realtimeFeedEditorDialog.open()
    }

    ColumnLayout {
        AccountLayout {
            Layout.fillWidth: true
            source: account.avatar
            handle: account.handle
        }
        RowLayout {
            Label {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Name") + " :"
            }
            TextField {
                id: ruleNameTextField
                Layout.preferredWidth: 200 * AdjustedValues.ratio
                font.pointSize: AdjustedValues.f10
                // placeholderText: qsTr("Name of rule")
            }
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
                    Layout.preferredWidth: 300 * AdjustedValues.ratio
                    Layout.preferredHeight: 300 * AdjustedValues.ratio
                    //ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    ListView {
                        id: editSelectorListView
                        clip: true
                        model: EditSelectorListModel {
                            id: editSelectorListModel
                            name: ruleNameTextField.text
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
                            displayType: model.displayType + ((model.listName.length > 0) ? (" : " + model.listName) : "")
                            useAppendButton: model.canHave
                            highlighted:  (model.index === editSelectorListView.currentIndex)
                            onClicked: {
                                editSelectorListView.currentIndex = model.index
                                editSelectorListView.updateDetailInfo(model.index)
                            }
                            onAppendChild: (row, type) => editSelectorListModel.appendChild(row, type)
                            onRemove: (row) => editSelectorListModel.remove(row)
                        }

                        function updateDetailInfo(index){
                            // image
                            hasImageCheckBox.checked = editSelectorListModel.item(index, EditSelectorListModel.HasImageRole)
                            imageCountComboBox.currentIndex = -1
                            imageCountComboBox.setByValue(editSelectorListModel.item(index, EditSelectorListModel.ImageCountRole))
                            // movie
                            hasMovieCheckBox.checked = editSelectorListModel.item(index, EditSelectorListModel.HasMovieRole)
                            movieCountComboBox.currentIndex = -1
                            movieCountComboBox.setByValue(editSelectorListModel.item(index, EditSelectorListModel.MovieCountRole))
                            // quote
                            hasQuoteCheckBox.checked = editSelectorListModel.item(index, EditSelectorListModel.HasQuoteRole)
                            quoteComboBox.currentIndex = -1
                            quoteComboBox.setByValue(editSelectorListModel.item(index, EditSelectorListModel.QuoteConditionRole))
                            // repost
                            isRepostCheckBox.checked = editSelectorListModel.item(index, EditSelectorListModel.IsRepostRole)
                            repostComboBox.currentIndex = -1
                            repostComboBox.setByValue(editSelectorListModel.item(index, EditSelectorListModel.RepostConditionRole))

                            // list
                            listComboBox.currentIndex = -1
                            listComboBox.setByValue(editSelectorListModel.item(index, EditSelectorListModel.ListUriRole))
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
                Layout.preferredWidth: 250 * AdjustedValues.ratio
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
            ColumnLayout {
                id: selectorDetailLayout
                Layout.preferredWidth: 250 * AdjustedValues.ratio
                spacing: 0
                visible: false
                property int adjustedPadding: 6 * AdjustedValues.ratio
                Label {
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("Detail")
                }
                Label {
                    visible: listComboBox.visible
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("List")
                }
                ComboBoxEx {
                    id: listComboBox
                    visible: (editSelectorListView.currentItem &&
                              editSelectorListView.currentItem.type === "list")
                    Layout.leftMargin: 10 * AdjustedValues.ratio
                    Layout.fillWidth: true
                    topPadding: parent.adjustedPadding
                    bottomPadding: parent.adjustedPadding
                    font.pointSize: AdjustedValues.f10
                    textRole: "name"
                    valueRole: "uri"
                    model: ListsListModel {
                        id: listsListModel
                        actor: realtimeFeedEditorDialog.account.did
                    }
                    onActivated: (index) => editSelectorListModel.updateList(editSelectorListView.currentIndex,
                                                                             currentText,
                                                                             currentValue)
                }
                RowLayout {
                    CheckBox {
                        id: hasImageCheckBox
                        topPadding: parent.adjustedPadding
                        bottomPadding: parent.adjustedPadding
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Image")
                        onCheckedChanged: {
                            if(checked){
                                hasMovieCheckBox.checked = false
                            }
                            editSelectorListModel.update(editSelectorListView.currentIndex,
                                                         EditSelectorListModel.HasImageRole,
                                                         checked)
                        }
                    }
                    ComboBoxEx {
                        id: imageCountComboBox
                        topPadding: parent.adjustedPadding
                        bottomPadding: parent.adjustedPadding
                        font.pointSize: AdjustedValues.f10
                        enabled: hasImageCheckBox.checked
                        model: ListModel {
                            ListElement { value: 0; text: qsTr("0") }
                            ListElement { value: 1; text: qsTr("1") }
                            ListElement { value: 2; text: qsTr("2") }
                            ListElement { value: 3; text: qsTr("3") }
                            ListElement { value: 4; text: qsTr("4") }
                            ListElement { value: -1; text: qsTr(">=1") }
                        }
                        onActivated: (index) => editSelectorListModel.update(editSelectorListView.currentIndex,
                                                                             EditSelectorListModel.ImageCountRole,
                                                                             currentValue)
                    }
                }
                RowLayout {
                    CheckBox {
                        id: hasMovieCheckBox
                        topPadding: parent.adjustedPadding
                        bottomPadding: parent.adjustedPadding
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Video")
                        onCheckedChanged: {
                            if(checked){
                                hasImageCheckBox.checked = false
                            }
                            editSelectorListModel.update(editSelectorListView.currentIndex,
                                                         EditSelectorListModel.HasMovieRole,
                                                         checked)
                        }
                    }
                    ComboBoxEx {
                        id: movieCountComboBox
                        topPadding: parent.adjustedPadding
                        bottomPadding: parent.adjustedPadding
                        font.pointSize: AdjustedValues.f10
                        enabled: hasMovieCheckBox.checked
                        model: ListModel {
                            ListElement { value: 0; text: qsTr("0") }
                            ListElement { value: 1; text: qsTr("1") }
                        }
                        onActivated: (index) => editSelectorListModel.update(editSelectorListView.currentIndex,
                                                                             EditSelectorListModel.MovieCountRole,
                                                                             currentValue)
                    }
                }
                RowLayout {
                    CheckBox {
                        id: hasQuoteCheckBox
                        topPadding: parent.adjustedPadding
                        bottomPadding: parent.adjustedPadding
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Quote")
                        onCheckedChanged: editSelectorListModel.update(editSelectorListView.currentIndex,
                                                                       EditSelectorListModel.HasQuoteRole,
                                                                       checked)
                    }
                    ComboBoxEx {
                        id: quoteComboBox
                        topPadding: parent.adjustedPadding
                        bottomPadding: parent.adjustedPadding
                        font.pointSize: AdjustedValues.f10
                        enabled: hasQuoteCheckBox.checked
                        model: ListModel {
                            ListElement { value: 0; text: qsTr("Only") }
                            ListElement { value: 1; text: qsTr("Exclude") }
                        }
                        onActivated: (index) => editSelectorListModel.update(editSelectorListView.currentIndex,
                                                                             EditSelectorListModel.QuoteConditionRole,
                                                                             currentValue)
                    }
                }
                RowLayout {
                    CheckBox {
                        id: isRepostCheckBox
                        topPadding: parent.adjustedPadding
                        bottomPadding: parent.adjustedPadding
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Repost")
                        onCheckedChanged: editSelectorListModel.update(editSelectorListView.currentIndex,
                                                                       EditSelectorListModel.IsRepostRole,
                                                                       checked)
                    }
                    ComboBoxEx {
                        id: repostComboBox
                        topPadding: parent.adjustedPadding
                        bottomPadding: parent.adjustedPadding
                        font.pointSize: AdjustedValues.f10
                        enabled: isRepostCheckBox.checked
                        model: ListModel {
                            ListElement { value: 0; text: qsTr("Only") }
                            ListElement { value: 1; text: qsTr("Exclude") }
                        }
                        onActivated: (index) => editSelectorListModel.update(editSelectorListView.currentIndex,
                                                                             EditSelectorListModel.RepostConditionRole,
                                                                             currentValue)
                    }
                }
                // CheckBox {
                //     topPadding: parent.adjustedPadding
                //     bottomPadding: parent.adjustedPadding
                // font.pointSize: AdjustedValues.f10
                //     text: qsTr("Contains hashtag")
                // }
                // TextField {
                //     Layout.leftMargin: AdjustedValues.s20
                //     topPadding: parent.adjustedPadding
                //     // bottomPadding: parent.adjustedPadding
                // font.pointSize: AdjustedValues.f10
                // }
                // CheckBox {
                //     topPadding: parent.adjustedPadding
                //     bottomPadding: parent.adjustedPadding
                // font.pointSize: AdjustedValues.f10
                //     text: qsTr("Contains word")
                // }
                // TextField {
                //     Layout.leftMargin: AdjustedValues.s20
                //     topPadding: parent.adjustedPadding
                //     // bottomPadding: parent.adjustedPadding
                // font.pointSize: AdjustedValues.f10
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
                enabled: editSelectorListModel.valid && ruleNameTextField.text.length > 0
                text: qsTr("Save")
                onClicked: {
                    editSelectorListModel.save(realtimeFeedEditorDialog.account.uuid)
                    realtimeFeedEditorDialog.accept()
                }
            }
        }

    }
}
