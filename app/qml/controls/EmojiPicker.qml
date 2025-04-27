import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.controls.emojilistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

Dialog {
    id: emojiPicker
    width: 450
    height: 500

    signal emojiSelected(string emoji, bool selected)

    property string searchText: ""
    property var selectedEmojis: []

    function openPicker(selected_emojis) {
        console.log("openPicker:" + selected_emojis)
        emojiPicker.selectedEmojis = selected_emojis
        emojiListModel.loadFrequentlyUsed()
        pickerListView.positionViewAtBeginning()
        emojiPicker.open()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8
        // padding: 8

        // TextField {
        //     id: searchField
        //     placeholderText: "🔍 検索"
        //     onTextChanged: root.searchText = text
        // }

        // Frequently used
        // Smileys & People
        // Animals & Nature
        // Food & Drink
        // Activity
        // Travel & Places
        // Objects
        // Symbols
        // Flags
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            Repeater {
                model: ListModel {
                    ListElement { icon: "🕒"; help: "Frequently used" }
                    ListElement { icon: "😀"; help: "Smileys & People" }
                    ListElement { icon: "👋"; help: "People & Body" }
                    ListElement { icon: "😺"; help: "Animals & Nature" }
                    ListElement { icon: "🍞"; help: "Food & Drink" }
                    ListElement { icon: "🌍"; help: "Travel & Places" }
                    ListElement { icon: "🎃"; help: "Activities" }
                    ListElement { icon: "👓"; help: "Objects" }
                    ListElement { icon: "📛"; help: "Symbols" }
                }
                TabButton {
                    text: model.icon
                    onClicked: {
                        console.log(emojiListModel.getGroupName(model.index) +
                                    ", " + emojiListModel.getGroupTopRow(model.index))
                        pickerListView.positionViewAtIndex(emojiListModel.getGroupTopRow(model.index),
                                                           ListView.Beginning)
                    }

                }
            }
        }

        ScrollView {
            id: pickerScrollView
            Layout.fillWidth: true
            Layout.fillHeight: true
            property int cellWidth: ((pickerScrollView.width) / emojiListModel.columnCount)
            function viewItem(obj){
                var scaled_pos = obj.y / pickerListView.implicitHeight
                // var new_pos = 1.0 - scaled_pos
                if(scaled_pos > (1.0 - pickerScrollView.ScrollBar.vertical.size)){
                    scaled_pos = 1.0 - pickerScrollView.ScrollBar.vertical.size
                }
                pickerScrollView.ScrollBar.vertical.position = scaled_pos
            }

            ListView {
                id: pickerListView
                spacing: 0
                clip: true
                onCurrentSectionChanged: {
                    tabBar.currentIndex = emojiListModel.getGroupIndex(currentSection)
                    console.log("currentSection:" + currentSection + ", index=" + tabBar.currentIndex)
                }
                model: EmojiListModel{
                    id: emojiListModel
                    columnCount: 9
                    selectedEmojis: emojiPicker.selectedEmojis
                }
                delegate: RowLayout {
                    id: pickerRowLayout
                    property var parentModel: model.emojis
                    property var parentEmojisSelected: model.emojisSelected
                    property var parentEmojisEnabled: model.emojisEnabled
                    spacing: 0
                    Repeater {
                        model: pickerRowLayout.parentModel
                        Button {
                            Layout.preferredWidth: pickerScrollView.cellWidth
                            topPadding: 0
                            bottomPadding: 0
                            topInset: 0
                            bottomInset: 0
                            flat: true
                            highlighted: pickerRowLayout.parentEmojisSelected[model.index]
                            enabled: pickerRowLayout.parentEmojisEnabled[model.index]
                            onClicked: {
                                emojiPicker.emojiSelected(modelData, highlighted)
                                emojiPicker.accept()
                                if(!highlighted){
                                    // 選択済み（つまり消すとき）は更新しない
                                    emojiListModel.setFrequentlyUsed(modelData)
                                }
                            }
                            Label {
                                anchors.centerIn: parent
                                font.pointSize: AdjustedValues.f14
                                text: modelData
                            }
                        }
                    }
                }
                section.property: "groupName"
                section.criteria: ViewSection.FullString
                section.delegate: Label {
                    required property string section
                    text: section
                    font.bold: true
                }
            }
        }
    }
}
