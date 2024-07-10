import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.chatlistmodel 1.0
import tech.relog.hagoromo.searchprofilelistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"
import "../compat"

Item {
    id: chatListView

    property string accountDid: ""   // 取得するユーザー

    property alias listView: rootListView
    property alias model: relayObject
    property string requestedConvoId: ""

    signal requestViewChatMessage(string convo_id, var dids)
    signal errorOccured(string code, string message)

    function resume(){
        console.log("chatListView: resume from " + requestedConvoId)
        if(requestedConvoId.length > 0){
            rootListView.model.updateRead(requestedConvoId, "")
        }
    }


    QtObject {
        id: relayObject
        function rowCount() {
            return rootListView.model.rowCount()
        }
        function setAccount(service, did, handle, email, accessJwt, refreshJwt) {
            accountDid = did
            searchProfileListModel.setAccount(service, did, handle, email, accessJwt, refreshJwt)
            rootListView.model.setAccount(service, did, handle, email, accessJwt, refreshJwt)
        }
        function getLatest() {
            rootListView.model.getLatest()
        }
    }


    ScrollView {
        id: chatListScrollView
        anchors.fill: parent
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        clip: true


        ListView {
            id: rootListView
            anchors.fill: parent
            anchors.rightMargin: parent.ScrollBar.vertical.width
            spacing: 5
            maximumFlickVelocity: AdjustedValues.maximumFlickVelocity
            model: ChatListModel {
                autoLoading: true
                loadingInterval: 30000  // 30s
                onErrorOccured: (code, message) => {
                                    if(code === "InvalidToken" && message === "Bad token scope"){
                                        errorMessageOnChatList.visible = true
                                    }else{
                                        chatListView.errorOccured(code, message)
                                    }
                                }
                onFinishUpdateRead: (success) => getLatest()
            }

            onMovementEnded: {
                if(atYEnd){
                    rootListView.model.getNext()
                }
            }

            header: ItemDelegate {
                width: rootListView.width
                height: AdjustedValues.h24
                display: AbstractButton.IconOnly
                icon.source: rootListView.model.running ? "" : "../images/expand_less.png"
                onClicked: rootListView.model.getLatest()

                BusyIndicator {
                    anchors.centerIn: parent
                    width: AdjustedValues.i24
                    height: AdjustedValues.i24
                    visible: rootListView.model.running
                }
            }
            footer: RowLayout {
                z: 3
                width: rootListView.width
                BusyIndicator {
                    Layout.fillWidth: true
                    Layout.preferredHeight: AdjustedValues.i24
                    visible: rootListView.model.running && rootListView.model.rowCount() > 0
                }
                IconButton {
                    Layout.preferredWidth: AdjustedValues.b55
                    Layout.alignment: Qt.AlignRight
                    Layout.rightMargin: 5
                    iconSource: "../images/add.png"
                    onClicked: startNewChatLayout.open()
                }
            }
            footerPositioning: ListView.OverlayFooter

            delegate: ClickableFrame {
                id: chatItemLayout
                clip: true
                topPadding: 10
                leftPadding: 10
                rightPadding: 2
                bottomPadding: 10
                enabled: !model.running

                property int layoutWidth: rootListView.width

                onClicked: {
                    requestedConvoId = model.id
                    chatListView.requestViewChatMessage(model.id, [])
                    rootListView.model.update(model.index, ChatListModel.UnreadCountRole, 0)
                }

                RowLayout {
                    AvatarImage {
                        id: postAvatarImage
                        Layout.preferredWidth: AdjustedValues.i36
                        Layout.preferredHeight: AdjustedValues.i36
                        source: model.memberAvatars.length > 0 ? model.memberAvatars[0] : "../images/account_icon.png"
                    }
                    ColumnLayout {
                        property int basisWidth: chatItemLayout.layoutWidth - chatItemLayout.leftPadding - chatItemLayout.rightPadding -
                                                 postAvatarImage.width - (parent.spacing * 2) - moreButton.width

                        RowLayout {
                            Layout.preferredWidth: parent.basisWidth
                            Label {
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                                font.pointSize: AdjustedValues.f10
                                text: model.memberDisplayNames.length > 0 ? model.memberDisplayNames[0] : ""
                            }
                            Image {
                                Layout.preferredWidth: AdjustedValues.i10
                                Layout.preferredHeight: AdjustedValues.i10
                                source: "../images/mute.png"
                                visible: model.muted
                                layer.enabled: true
                                layer.effect: ColorOverlayC {
                                    color: Material.foreground
                                }
                            }
                            Label {
                                Layout.preferredWidth: implicitWidth
                                Layout.minimumWidth: implicitWidth
                                font.pointSize: AdjustedValues.f8
                                color: Material.color(Material.Grey)
                                text: model.lastMessageSentAt
                            }
                        }
                        Label {
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                            font.pointSize: AdjustedValues.f8
                            color: Material.color(Material.Grey)
                            text: model.memberHandles.length > 0 ? ("@" + model.memberHandles[0]) : ""
                        }

                        Label {
                            Layout.preferredWidth: parent.basisWidth
                            font.pointSize: AdjustedValues.f10
                            color: Material.color(Material.Grey)
                            text: model.lastMessageText
                            textFormat: Text.StyledText
                            wrapMode: Text.WrapAnywhere

                            Rectangle {
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                width: childrenRect.width * 2
                                height: childrenRect.height
                                radius: height / 2
                                visible: model.unreadCountRole > 0
                                color: Material.accentColor
                                Label {
                                    id: unreadCountLabel
                                    x: width / 2
                                    font.pointSize: AdjustedValues.f8
                                    text: model.unreadCountRole
                                }
                            }
                        }
                    }
                    IconButton {
                        id: moreButton
                        Layout.preferredHeight: AdjustedValues.b24
                        Layout.alignment: Qt.AlignVCenter
                        iconSource: "../images/more.png"
                        iconSize: AdjustedValues.i16
                        foreground: Material.color(Material.Grey)
                        flat: true
                        onClicked: myMorePopup.open()
                        Menu {
                            id: myMorePopup
                            MenuItemEx {
                                text: model.muted ? qsTr("Unmute conversation") : qsTr("Mute conversation")
                                icon.source: "../images/mute.png"
                                onTriggered: rootListView.model.mute(model.index)
                            }
                            MenuItemEx {
                                text: qsTr("Leave conversation")
                                icon.source: "../images/leave.png"
                                onTriggered: rootListView.model.leave(model.index)
                            }
                        }
                        BusyIndicator {
                            anchors.fill: parent
                            visible: model.running
                        }
                    }
                }
            }
        }
    }
    Pane {
        id: startNewChatLayout
        x: 0
        y: parent.height
        width: parent.width
        height: parent.height
        leftPadding: 0
        rightPadding: 0

        function open(){
            y = 0
            searchTextArea.forceActiveFocus()
        }
        function close(){
            y = parent.height
            searchTextArea.text = ""
        }

        Shortcut {  // close
            enabled: chatListView.visible && (startNewChatLayout.y === 0) && searchTextArea.focus
            sequence: "Esc"
            onActivated: startNewChatLayout.close()
        }
        Behavior on y {
            NumberAnimation {

                duration: 200
                // easing.type: Easing.InOutQuad
            }
        }

        ColumnLayout {
            anchors.fill: parent
            RowLayout {
                Label {
                    Layout.fillWidth: true
                    font.bold: true
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("Start a new chat")
                }
                IconButton {
                    flat: true
                    iconSource: "../images/close.png"
                    onClicked: startNewChatLayout.close()
                }
            }
            RowLayout {
                Image {
                    id: iconImage
                    Layout.preferredWidth: AdjustedValues.i18
                    Layout.preferredHeight: AdjustedValues.i18
                    Layout.alignment: Qt.AlignVCenter
                    Layout.leftMargin: 5
                    layer.enabled: true
                    layer.effect: ColorOverlayC {
                        color: Material.foreground
                    }
                    source: "../images/search.png"
                }
                TextArea {
                    id: searchTextArea
                    Layout.fillWidth: parent
                    Layout.leftMargin: 5
                    Layout.rightMargin: 5
                    wrapMode: Text.WrapAnywhere
                    selectByMouse: true
                    font.pointSize: AdjustedValues.f10
                    placeholderText: qsTr("Search")
                    onTextChanged: {
                        if(text.length > 0){
                            searchProfileListModel.getSuggestion(text, 10)
                        }else{
                            searchProfileListModel.clear()
                        }
                    }
                }
            }
            SuggestionProfileListView {
                id: suggestionProfileListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                accountDid: chatListView.accountDid
                model: SearchProfileListModel {
                    id: searchProfileListModel
                    enabledSuggestion: true
                    onErrorOccured: (code, message) => chatListView.errorOccured(code, message)
                }
                onSelectedProfile: (did) => {
                                       startNewChatLayout.close()
                                       chatListView.requestViewChatMessage("", [did])
                                   }
            }
        }
    }
    ChatErrorMessage {
        id: errorMessageOnChatList
        anchors.fill: parent
        visible: false
    }
}
