import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.followslistmodel 1.0
import tech.relog.hagoromo.searchprofilelistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

ListView {
    id: mentionSuggestionView
    height: count * (AdjustedValues.i12 + 5)
    clip: true
    visible: searchProfileListModel.count > 0

    signal selected(string handle)

    function setAccount(service, did, handle, accessJwt){
        searchProfileListModel.setAccount(service, did, handle, "", accessJwt, "")
    }

    function reload(text){
        var handle_part = searchProfileListModel.extractHandleBlock(text)
        if(handle_part.length === 0){
            searchProfileListModel.clear()
        }else{
            console.log(handle_part)
            searchProfileListModel.getSuggestion(handle_part, 6)
        }
    }
    function replaceText(text, current_position, handle){
        return searchProfileListModel.replaceText(text, current_position, handle)
    }

    function accept(){
        if(currentIndex >= 0){
            selected(searchProfileListModel.item(currentIndex, FollowsListModel.HandleRole))
        }
        searchProfileListModel.clear()
    }
    function up(){
        if(currentIndex > 0){
            currentIndex -= 1
        }
    }
    function down(){
        if(currentIndex < (count - 1)){
            currentIndex += 1
        }
    }

    model: SearchProfileListModel {
        id: searchProfileListModel
        autoLoading: false
        onErrorOccured: (code, message) => console.log(code + " : " + message)
    }
    delegate: ItemDelegate {
        width: mentionSuggestionView.width
        height: AdjustedValues.i12 + 5
        highlighted: ListView.isCurrentItem
        RowLayout {
            id: userLayout
            width: parent.width
            height: parent.height
            AvatarImage {
                id: avatarImage
                Layout.preferredWidth: AdjustedValues.i12
                Layout.preferredHeight: AdjustedValues.i12
                Layout.alignment: Qt.AlignVCenter
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
