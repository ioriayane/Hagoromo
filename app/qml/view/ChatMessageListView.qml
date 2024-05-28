import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"

Item {
    id: chatMessageListView

    property string hoveredLink: ""
    property string accountDid: ""   // 取得するユーザー

    property alias listView: rootListView
    property alias model: rootListView.model
    property alias errorMessageOnChatMessageList: errorMessageOnChatMessageList

    signal requestReportMessage(string did, string convo_id, string message_id)
    signal requestViewProfile(string did)
    signal requestViewSearchPosts(string text)
    signal requestAddMutedWord(string text)

    function finishSent(success) {
        if(success){
            messageTextArea.text = ""
        }else{
            console.log("Fail send")
        }
    }
    function displayLink(url){
        if(url === undefined || url.indexOf("did:") === 0 || url.indexOf("search://") === 0){
            hoveredLink = ""
        }else{
            hoveredLink = url
        }
    }

    Shortcut {  // Post
        enabled: chatMessageListView.visible && sendButton.enabled && messageTextArea.focus
        sequence: "Ctrl+Return"
        onActivated: sendButton.clicked()
    }
    QtObject {
        id: relayObject
        function rowCount() {
            return rootListView.model.rowCount();
        }
        function setAccount(service, did, handle, email, accessJwt, refreshJwt) {
            rootListView.model.setAccount(service, did, handle, email, accessJwt, refreshJwt)
        }
        function getLatest() {
            rootListView.model.getLatest()
        }
    }

    ColumnLayout {
        anchors.fill: parent

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            clip: true

            ListView {
                id: rootListView
                anchors.fill: parent
                anchors.rightMargin: parent.ScrollBar.vertical.width
                spacing: 5
                maximumFlickVelocity: AdjustedValues.maximumFlickVelocity
                verticalLayoutDirection: ListView.BottomToTop

                onMovementEnded: {
                    if(atYEnd){
                        rootListView.model.getNext()
                    }
                }

                header: BusyIndicator {
                    width: rootListView.width
                    height: AdjustedValues.i24
                    visible: rootListView.model.running &&
                             rootListView.model.rowCount() === 0 &&
                             !rootListView.model.ready
                }
                // footer: BusyIndicator {
                //     width: rootListView.width
                //     height: AdjustedValues.i24
                //     visible: rootListView.model.running && rootListView.model.rowCount() > 0
                // }

                delegate: Control {
                    id: chatItemLayout
                    clip: true
                    width: rootListView.width
                    height: childrenRect.height
                    topPadding: 10
                    leftPadding: 10
                    rightPadding: 10
                    // bottomPadding: 10

                    property int layoutWidth: rootListView.width
                    property bool me: model.senderDid === accountDid

                    function openLink(url){
                        if(url.indexOf("did:") === 0){
                            chatMessageListView.requestViewProfile(url)
                        }else if(url.indexOf("search://") === 0){
                            // tagMenu.x = recrdTextMouseArea.mouseX
                            // tagMenu.y = recrdTextMouseArea.mouseY
                            tagMenu.tagText = url.substring(9)
                            if(tagMenu.tagText.charAt(0) !== "#"){
                                tagMenu.tagText = "#" + tagMenu.tagText
                            }
                            tagMenu.open()
                        }else{
                            Qt.openUrlExternally(url)
                        }
                    }

                    states: [
                        State {
                            when: chatItemLayout.me
                            AnchorChanges {
                                target: messageItemLayout
                                anchors.right: chatItemLayout.right
                                anchors.left: undefined
                            }
                            AnchorChanges {
                                target: moreButton
                                anchors.right: messageBubble.left
                                anchors.left: undefined
                            }
                        }
                    ]
                    RowLayout {
                        id: messageItemLayout
                        anchors.left: parent.left
                        anchors.leftMargin: 5
                        anchors.rightMargin: 5

                        AvatarImage {
                            id: postAvatarImage
                            Layout.preferredWidth: AdjustedValues.i24
                            Layout.preferredHeight: AdjustedValues.i24
                            Layout.alignment: Qt.AlignTop
                            source: model.senderAvatar
                            visible: !chatItemLayout.me
                        }
                        ColumnLayout {
                            property int basisWidth: chatItemLayout.layoutWidth - chatItemLayout.leftPadding - chatItemLayout.rightPadding -
                                                     postAvatarImage.width - parent.spacing
                            MessageBubble {
                                id: messageBubble
                                Layout.alignment: chatItemLayout.me ? Qt.AlignRight : Qt.AlignLeft
                                Layout.maximumWidth: parent.basisWidth * 0.7
                                font.pointSize: AdjustedValues.f10
                                text: model.text
                                fromRight: chatItemLayout.me

                                onLinkActivated: (url) => chatItemLayout.openLink(url)
                                onHoveredLinkChanged: displayLink(hoveredLink)

                                HashTagMenu {
                                    id: tagMenu
                                    logMode: false
                                    onRequestViewSearchPosts: (text) => chatMessageListView.requestViewSearchPosts(text)
                                    onRequestAddMutedWord: (text) => chatMessageListView.requestAddMutedWord(text)
                                }
                                MoreButton {
                                    id: moreButton
                                    anchors.left: parent.right
                                    anchors.leftMargin: 2
                                    anchors.rightMargin: 2
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: AdjustedValues.b24
                                    height: AdjustedValues.b24
                                    hoverEnabled: true
                                    opacity: hovered ? 1.0 : 0.0
                                    onClicked: morePopup.open()
                                }
                            }
                            Label {
                                Layout.alignment: chatItemLayout.me ? Qt.AlignRight : Qt.AlignLeft
                                font.pointSize: AdjustedValues.f8
                                color: Material.color(Material.Grey)
                                text: model.sentAt
                            }
                        }
                        Menu {
                            id: morePopup
                            width: deleteMenuItem.implicitWidth > reportMenuItem.implicitWidth ?
                                       deleteMenuItem.implicitWidth : reportMenuItem.implicitWidth
                            MenuItem {
                                id: deleteMenuItem
                                text: qsTr("Delete for me")
                                icon.source: "../images/delete.png"
                                // onTriggered: rootListView.model.mute(model.index)
                            }
                            MenuItem {
                                id: reportMenuItem
                                enabled: !chatItemLayout.me
                                text: qsTr("Report message")
                                icon.source: "../images/report.png"
                                onTriggered: chatMessageListView.requestReportMessage(model.senderDid,
                                                                                      rootListView.model.convoId,
                                                                                      model.id)
                            }
                        }

                    }
                }
            }
        }
        RowLayout {
            TextArea {
                id: messageTextArea
                Layout.fillWidth: parent
                Layout.leftMargin: 5
                enabled: !rootListView.model.runSending
                wrapMode: Text.WrapAnywhere
                selectByMouse: true
                font.pointSize: AdjustedValues.f10
                placeholderText: qsTr("Write a message")
            }
            IconButton {
                id: sendButton
                font.pointSize: AdjustedValues.f10
                iconSource: "../images/send.png"
                enabled: messageTextArea.text.length > 0 && !rootListView.model.runSending && rootListView.model.ready
                onClicked: {
                    rootListView.model.send(messageTextArea.text)
                }
                BusyIndicator {
                    anchors.fill: parent
                    visible: rootListView.model.runSending
                }
            }
        }
    }
    ChatErrorMessage {
        id: errorMessageOnChatMessageList
        anchors.fill: parent
        visible: false
    }
}
