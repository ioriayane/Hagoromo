import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.followslistmodel 1.0
import tech.relog.hagoromo.searchprofilelistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

Rectangle {
    id: mentionSuggestionView
    height: listView.count * (AdjustedValues.i12 + 5) + listView.anchors.margins * 2
    visible: listView.count > 0
    color: Material.backgroundColor

    property string handlePart: ""

    signal selected(string handle)

    function setAccount(uuid){
        searchProfileListModel.setAccount(uuid)
    }

    function reload(text){
        handlePart = searchProfileListModel.extractHandleBlock(text)
        if(handlePart.length === 0){
            searchProfileListModel.clear()
        }else{
            console.log(handlePart)
            searchProfileListModel.getSuggestion(handlePart, 6)
        }
    }
    function replaceText(text, current_position, handle){
        return searchProfileListModel.replaceText(text, current_position, handle)
    }

    function clear(){
        searchProfileListModel.clear()
    }

    function accept(){
        if(listView.currentIndex >= 0){
            selected(searchProfileListModel.item(listView.currentIndex, FollowsListModel.HandleRole))
        }
        searchProfileListModel.clear()
    }
    function up(){
        if(listView.currentIndex > 0){
            listView.currentIndex -= 1
        }
    }
    function down(){
        if(listView.currentIndex < (listView.count - 1)){
            listView.currentIndex += 1
        }
    }

    ListView {
        id: listView
        anchors.fill: parent
        anchors.margins: 2
        clip: true
        maximumFlickVelocity: AdjustedValues.maximumFlickVelocity

        model: SearchProfileListModel {
            id: searchProfileListModel
            autoLoading: false
            enabledSuggestion: mentionSuggestionView.handlePart.length > 0
            onErrorOccured: (code, message) => console.log(code + " : " + message)
        }
        delegate: ItemDelegate {
            width: mentionSuggestionView.width
            height: AdjustedValues.i12 + 5
            highlighted: ListView.isCurrentItem

            RowLayout {
                id: userLayout
                anchors.fill: parent
                anchors.leftMargin: 5
                anchors.rightMargin: 5
                AvatarImage {
                    id: avatarImage
                    Layout.preferredWidth: AdjustedValues.i12
                    Layout.preferredHeight: AdjustedValues.i12
                    Layout.alignment: Qt.AlignVCenter
                    source: model.avatar
                }
                Label {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.maximumWidth: mentionSuggestionView.width - (
                                             avatarImage.width + handleLabel.width + userLayout.spacing * 3
                                             )
                    font.pointSize: AdjustedValues.f8
                    text: model.displayName
                    elide: Text.ElideRight
                }
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                }
                Label {
                    id: handleLabel
                    Layout.alignment: Qt.AlignVCenter
                    font.pointSize: AdjustedValues.f8
                    text: "@" + model.handle
                }
            }
            onClicked: {
                mentionSuggestionView.selected(model.handle)
                searchProfileListModel.clear()
            }
        }
    }
}
