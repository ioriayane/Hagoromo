import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"

ColumnLayout {
    id: chatMessageListView

    property string hoveredLink: ""
    property string accountDid: ""   // 取得するユーザー

    property alias listView: rootListView
    property alias model: rootListView.model

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
                visible: rootListView.model.running && rootListView.model.rowCount() === 0
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
                            Layout.maximumWidth: parent.basisWidth * 0.8
                            Layout.alignment: chatItemLayout.me ? Qt.AlignRight : Qt.AlignLeft
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
                        }
                        Label {
                            Layout.alignment: chatItemLayout.me ? Qt.AlignRight : Qt.AlignLeft
                            font.pointSize: AdjustedValues.f8
                            color: Material.color(Material.Grey)
                            text: model.sentAt
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
