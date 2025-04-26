import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.userpost 1.0
import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"

Item {
    id: chatMessageListView

    property string hoveredLink: ""
    property string accountDid: ""   // 取得するユーザー

    property alias listView: rootListView
    // property alias model: rootListView.model
    property alias model: relayObject
    property alias errorMessageOnChatMessageList: errorMessageOnChatMessageList

    signal requestReportMessage(string did, string convo_id, string message_id)
    signal requestViewThread(string uri)
    signal requestViewImages(int index, var paths, var alts)
    signal requestViewProfile(string did)
    signal requestViewSearchPosts(string text)
    signal requestAddMutedWord(string text)

    function finishSent(success) {
        if(success){
            messageTextArea.text = ""
            quoteUrlTextArea.text = ""
            quoteUrlInputLayout.visible = false
            userPost.clear()
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
        function setAccount(uuid) {
            rootListView.model.setAccount(uuid)
            userPost.setAccount(uuid)
        }
        function getLatest() {
            rootListView.model.getLatest()
        }
    }
    UserPost {
        id: userPost
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        ClickableFrame {
            Layout.fillWidth: true
            Layout.topMargin: 1
            contentWidth: contentRootLayout.implicitWidth
            contentHeight: contentRootLayout.implicitHeight
            topPadding: 5
            bottomPadding: 5
            visible: rootListView.model.memberHandles.length > 0
            RowLayout {
                id: contentRootLayout
                AvatarImage {
                    id: memberAvatarsImage
                    Layout.preferredWidth: AdjustedValues.i24
                    Layout.preferredHeight: AdjustedValues.i24
                    source: rootListView.model.memberAvatars.length > 0 ?
                                rootListView.model.memberAvatars[0] : "../images/account_icon.png"
                }
                ColumnLayout {
                    spacing: 1
                    Label {
                        font.pointSize: AdjustedValues.f8
                        text: rootListView.model.memberDisplayNames.length > 0 ?
                                  rootListView.model.memberDisplayNames[0] : ""
                    }
                    Label {
                        font.pointSize: AdjustedValues.f8
                        color: Material.color(Material.Grey)
                        text: rootListView.model.memberHandles.length > 0 ?
                                  ("@" + rootListView.model.memberHandles[0]) : ""
                    }
                }
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
                                target: emojiButton
                                anchors.right: messageBubble.left
                                anchors.left: undefined
                            }
                            AnchorChanges {
                                target: moreButton
                                anchors.right: emojiButton.left
                                anchors.left: undefined
                            }
                        }
                    ]
                    ColumnLayout {
                        id: messageItemLayout
                        anchors.left: parent.left
                        anchors.leftMargin: 5
                        anchors.rightMargin: 5

                        CoverFrame {
                            id: quoteFilterFrame
                            Layout.preferredWidth: chatItemLayout.width * 0.8
                            visible: model.quoteFilterMatched && !model.quoteRecordBlocked
                            labelText: qsTr("Quoted content warning")
                        }
                        QuoteRecord {
                            id: quoteRecordFrame
                            Layout.alignment: chatItemLayout.me ? Qt.AlignRight : Qt.AlignLeft
                            Layout.preferredWidth: chatItemLayout.width * 0.8
                            visible: model.hasQuoteRecord &&
                                     quoteFilterFrame.showContent &&
                                     !model.quoteRecordBlocked

                            onClicked: (mouse) => {
                                           if(model.quoteRecordUri.length > 0){
                                               requestViewThread(model.quoteRecordUri)
                                           }
                                       }
                            quoteRecordAvatarImage.source: model.quoteRecordAvatar
                            quoteRecordAuthor.displayName: model.quoteRecordDisplayName
                            quoteRecordAuthor.handle: model.quoteRecordHandle
                            quoteRecordAuthor.indexedAt: model.quoteRecordIndexedAt
                            quoteRecordRecordText.text: model.quoteRecordRecordText
                            quoteRecordImagePreview.layoutType: 1
                            quoteRecordImagePreview.embedImages: model.quoteRecordEmbedImages
                            quoteRecordImagePreview.embedAlts: model.quoteRecordEmbedImagesAlt
                            quoteRecordImagePreview.onRequestViewImages: (index) => requestViewImages(index, model.quoteRecordEmbedImagesFull, model.quoteRecordEmbedImagesAlt)
                        }
                        Frame {
                            id: blockedQuoteFrame
                            Layout.preferredWidth: chatItemLayout.width * 0.8
                            visible: model.quoteRecordBlocked
                            Label {
                                font.pointSize: AdjustedValues.f10
                                text: qsTr("Blocked")
                            }
                        }

                        RowLayout {
                            Layout.alignment: chatItemLayout.me ? Qt.AlignRight : Qt.AlignLeft

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
                                    visible: text.length > 0 || !quoteRecordFrame.visible
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
                                        id: emojiButton
                                        anchors.left: parent.right
                                        anchors.leftMargin: 2
                                        anchors.rightMargin: 2
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: AdjustedValues.b24
                                        height: AdjustedValues.b24
                                        hoverEnabled: true
                                        opacity: (hovered || model.running || moreButton.hovered) ? 1.0 : 0.0
                                        iconSource: "../images/smile.png"
                                        enabled: model.canReaction
                                        onClicked: {
                                            emojiPicker.messageIndex = model.index
                                            emojiPicker.open()
                                        }
                                    }
                                    MoreButton {
                                        id: moreButton
                                        anchors.left: emojiButton.right
                                        anchors.leftMargin: 2
                                        anchors.rightMargin: 2
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: AdjustedValues.b24
                                        height: AdjustedValues.b24
                                        hoverEnabled: true
                                        opacity: (hovered || model.running || emojiButton.hovered) ? 1.0 : 0.0
                                        onClicked: morePopup.open()
                                        BusyIndicator {
                                            anchors.fill: parent
                                            visible: model.running
                                        }
                                    }
                                }
                                Label {
                                    Layout.alignment: chatItemLayout.me ? Qt.AlignRight : Qt.AlignLeft
                                    Layout.topMargin: 0
                                    font.pointSize: AdjustedValues.f8
                                    text: model.reactionEmojis
                                }
                                Label {
                                    Layout.alignment: chatItemLayout.me ? Qt.AlignRight : Qt.AlignLeft
                                    font.pointSize: AdjustedValues.f8
                                    color: Material.color(Material.Grey)
                                    text: model.sentAt
                                }
                            }
                            MenuEx {
                                id: morePopup
                                Action {
                                    icon.source: "../images/copy.png"
                                    text: qsTr("Copy message")
                                    onTriggered: systemTool.copyToClipboard(model.textPlain)
                                }
                                Action {
                                    text: qsTr("Delete for me")
                                    icon.source: "../images/delete.png"
                                    onTriggered: rootListView.model.deleteMessage(model.index)
                                }
                                Action {
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
        }
        ColumnLayout {
            RowLayout {
                visible: userPost.cid.length > 0
                QuoteRecord {
                    id: sendPostFrame
                    Layout.fillWidth: true

                    quoteRecordAvatarImage.source: userPost.authorAvatar
                    quoteRecordAuthor.displayName: userPost.authorDisplayName
                    quoteRecordAuthor.handle: userPost.authorHandle
                    quoteRecordAuthor.indexedAt: userPost.indexedAt
                    quoteRecordRecordText.text: userPost.recordText
                    quoteRecordImagePreview.layoutType: 1
                    quoteRecordImagePreview.embedImages: userPost.embedImages
                    quoteRecordImagePreview.embedAlts: userPost.embedImagesAlt
                }
                IconButton {
                    id: deleteSendPostButton
                    Layout.alignment: Qt.AlignTop
                    font.pointSize: AdjustedValues.f10
                    iconSource: "../images/close.png"
                    enabled: userPost.cid.length > 0
                    flat: true
                    onClicked: userPost.clear()
                }
            }

            RowLayout {
                id: quoteUrlInputLayout
                visible: false
                TextArea {
                    id: quoteUrlTextArea
                    Layout.fillWidth: parent
                    Layout.leftMargin: 5
                    enabled: !rootListView.model.runSending
                    selectByMouse: true
                    font.pointSize: AdjustedValues.f10
                    placeholderText: qsTr("Post url or at-uri")
                }
                IconButton {
                    id: addPostButton
                    font.pointSize: AdjustedValues.f10
                    iconSource: "../images/add.png"
                    enabled: !rootListView.model.runSending &&
                             rootListView.model.ready &&
                             quoteUrlTextArea.text.length > 0 &&
                             !userPost.running
                    onClicked: userPost.getPost(quoteUrlTextArea.text)
                    BusyIndicator {
                        anchors.fill: parent
                        visible: userPost.running
                    }
                }
            }
            RowLayout {
                IconButton {
                    id: addButton
                    font.pointSize: AdjustedValues.f10
                    iconSource: "../images/add.png"
                    flat: true
                    enabled: !rootListView.model.runSending &&
                             rootListView.model.ready
                    onClicked: {
                        quoteUrlInputLayout.visible = !quoteUrlInputLayout.visible
                    }
                }
                TextArea {
                    id: messageTextArea
                    Layout.fillWidth: parent
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
                    enabled: (messageTextArea.text.length > 0 || userPost.cid.length > 0) &&
                             !rootListView.model.runSending &&
                             rootListView.model.ready &&
                             !userPost.running
                    onClicked: {
                        rootListView.model.send(messageTextArea.text, userPost.uri, userPost.cid)
                    }
                    BusyIndicator {
                        anchors.fill: parent
                        visible: rootListView.model.runSending
                    }
                }
            }
        }
    }
    ChatErrorMessage {
        id: errorMessageOnChatMessageList
        anchors.fill: parent
        visible: false
    }
    EmojiPicker {
        id: emojiPicker
        property int messageIndex: -1
        onEmojiSelected: (emoji) => {
                             rootListView.model.addReaction(messageIndex, emoji)
                         }
    }
}
