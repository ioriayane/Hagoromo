pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import Qt.labs.platform as P

import tech.relog.hagoromo.recordoperator 1.0
import tech.relog.hagoromo.draftoperator 1.0
import tech.relog.hagoromo.accountlistmodel 1.0
import tech.relog.hagoromo.controls.languagelistmodel 1.0
import tech.relog.hagoromo.externallink 1.0
import tech.relog.hagoromo.feedgeneratorlink 1.0
import tech.relog.hagoromo.controls.embedimagelistmodel 1.0
import tech.relog.hagoromo.listlink 1.0
import tech.relog.hagoromo.postlink 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../parts"

Item {
    id: postDialogItem

    property int dialog_no: -1
    property int dialog_default_x: -1
    property int dialog_default_y: -1
    property alias dialog_x: postDialogPosition.x   // xは位置決め用Item
    property alias dialog_y: postDialogPosition.y   // yは位置決め用Item
    property alias dialog_z: postDialog.z   // zはダイアログそのもの
    property real basisHeight: parentHeight * 0.9 - postDialog.topPadding - postDialog.bottomPadding
    property int parentWidth: 800
    property int parentHeight: 600
    property int viewIndex: 0
    property int bottomLine: 600
    property bool viewingProgress: progressFrame.visible

    property alias accountModel: accountCombo.model

    property string defaultAccountUuid: ""
    property string postType: "normal"  // normal, reply, quote

    property string replyCid: ""
    property string replyUri: ""
    property string replyRootCid: ""
    property string replyRootUri: ""
    property string replyAvatar: ""
    property string replyDisplayName: ""
    property string replyHandle: ""
    property string replyIndexedAt: ""
    property string replyText: ""

    property bool quoteValid: (quoteUri.startsWith("at://") && quoteCid.length > 0)
    property string quoteCid: ""
    property string quoteUri: ""
    property string quoteAvatar: ""
    property string quoteDisplayName: ""
    property string quoteHandle: ""
    property string quoteIndexedAt: ""
    property string quoteText: ""

    property alias postText: postText
    property alias recordOperator: createRecord

    signal errorOccured(string account_uuid, string code, string message)
    signal closed()
    signal closedDialog()
    signal changeActiveDialog(int dialog_no, bool active)

    function open(){
        var tmp_x = (postDialogItem.parentWidth - postDialog.width) * 0.5
        var tmp_y = (postDialogItem.parentHeight - scrollView.implicitHeight - postDialog.topPadding - postDialog.bottomPadding) * 0.5
        var offset = AdjustedValues.s15
        if(postDialogItem.dialog_default_x >= 0 &&
                postDialogItem.dialog_default_y >= 0 &&
                (postDialogItem.dialog_default_x + postDialog.width + offset) <= postDialogItem.parentWidth &&
                (postDialogItem.dialog_default_y + postDialog.height + offset) <= postDialogItem.parentHeight){
            tmp_x = postDialogItem.dialog_default_x + offset
            tmp_y = postDialogItem.dialog_default_y + offset
        }
        postDialogPosition.x = tmp_x
        postDialogPosition.y = tmp_y
        postDialog.open()
    }
    function close() {
        visible = false
        closedDialog()
        closed()
    }

    function openWithFiles(urls){
        if(embedImageListModel.append(urls)){
            postDialogItem.open()
        }else{
            close()
        }
    }

    Frame {
        id: progressFrame
        x: postDialogItem.parentWidth - width - 5
        y: postDialogItem.bottomLine - ((height + 5) * (postDialogItem.viewIndex + 1))
        contentWidth: progressLayout.width
        contentHeight: progressLayout.height
        visible: (createRecord.running && createRecord.progressMessage.length > 0) ||
                 (draftOperator.running && draftOperator.progressMessage.length > 0)
        background: Rectangle {
            radius: 3
            border.width: 1
            border.color: Material.frameColor
            color: Material.backgroundColor
        }
        ColumnLayout {
            id: progressLayout
            width: 300 * AdjustedValues.ratio
            Label {
                Layout.fillWidth: true
                Layout.maximumWidth: parent.width
                font.pointSize: AdjustedValues.f10
                elide: Text.ElideRight
                text: postText.text.split("\n")[0]
                clip: true
            }
            ProgressBar {
                Layout.fillWidth: true
                indeterminate: true
            }
            Label {
                id: progressLabel
                Layout.fillWidth: true
                Layout.maximumWidth: parent.width
                font.pointSize: AdjustedValues.f8
                text: draftOperator.running ? draftOperator.progressMessage : createRecord.progressMessage
                color: Material.theme === Material.Dark ? Material.foreground : "white"
            }
        }
    }

    Item {
        id: postDialogPosition
        width: postDialog.width
        height: postDialog.height
        Dialog {
            id: postDialog
            modal: false
            closePolicy: Popup.NoAutoClose
            topPadding: 20
            bottomPadding: 20
            rightPadding: 0
            focus: true
            onActiveFocusChanged: {
                console.log("Focus of dialog changed(" + postDialogItem.dialog_no + "):" + activeFocus)
                postDialogItem.changeActiveDialog(postDialogItem.dialog_no, activeFocus)
            }

            background: Rectangle {
                border.color: postDialog.activeFocus ? Material.color(Material.Grey, Material.Shade600) : Material.dialogColor
                color: Material.dialogColor
                radius: Material.dialogRoundedScale
                MouseArea {
                    id: dragArea
                    anchors.fill: parent
                    cursorShape: Qt.SizeAllCursor
                    drag.target: postDialogPosition
                    drag.minimumX: 0
                    drag.minimumY: 0
                    drag.maximumX: postDialogItem.parentWidth - postDialogPosition.width
                    drag.maximumY: postDialogItem.parentHeight - postDialogPosition.height
                    onPressed: postText.forceActiveFocus()
                }
            }

            onOpened: {
                var i = postDialogItem.accountModel.indexAt(postDialogItem.defaultAccountUuid)
                accountCombo.currentIndex = -1
                if(i >= 0){
                    accountCombo.currentIndex = i
                } else {
                    accountCombo.currentIndex = postDialogItem.accountModel.getMainAccountIndex()
                }
                postText.forceActiveFocus()
            }
            onClosed: postDialogItem.closedDialog()

            Shortcut {  // Post
                enabled: postDialog.visible && postButton.enabled && postText.focus && postDialog.activeFocus
                sequence: "Ctrl+Return"
                onActivated: postButton.clicked()
            }
            Shortcut {  // Close
                // DialogのclosePolicyでEscで閉じられるけど、そのうち編集中の確認ダイアログを
                // 入れたいので別でイベント処理をする。onClosedで閉じるをキャンセルできなさそうなので。
                enabled: postDialog.visible && ! postButton.enabled && postDialog.activeFocus
                sequence: "Esc"
                onActivated: postDialogItem.close()
            }

            SystemTool {
                id: systemTool
            }
            RecordOperator {
                id: createRecord
                onFinished: (success) => {
                    if(success){
                        // postText.clear()
                        postDialogItem.closed()
                    }
                }
                onErrorOccured: (code, message) => {
                    postDialog.open()
                    var row = accountCombo.currentIndex;
                    postDialogItem.errorOccured(postDialogItem.accountModel.item(row, AccountListModel.UuidRole), code, message)
                }
            }
            DraftOperator {
                id: draftOperator
                onFinishedCreateDraft: (success, id) => {
                    if(success){
                        postDialogItem.closed()
                    }
                }
                onErrorOccured: (code, message) => {
                    postDialog.open()
                    var row = accountCombo.currentIndex;
                    postDialogItem.errorOccured(postDialogItem.accountModel.item(row, AccountListModel.UuidRole), code, message)
                }
            }

            LanguageListModel {
                id: languageListModel
            }
            ExternalLink {
                id: externalLink
            }
            FeedGeneratorLink {
                id: feedGeneratorLink
            }
            ListLink {
                id: listLink
            }
            PostLink {
                id: postLink
                onValidChanged: {
                    if(postLink.valid){
                        postDialogItem.quoteCid = postLink.cid
                        postDialogItem.quoteUri = postLink.uri
                        postDialogItem.quoteAvatar = postLink.avatar
                        postDialogItem.quoteDisplayName = postLink.displayName
                        postDialogItem.quoteHandle = postLink.creatorHandle
                        postDialogItem.quoteIndexedAt = postLink.indexedAt
                        postDialogItem.quoteText = postLink.text
                    }
                }
            }

            ScrollView {
                id: scrollView
                implicitWidth: mainLayout.width + postDialog.leftPadding
                implicitHeight: (mainLayout.height > postDialogItem.basisHeight) ? postDialogItem.basisHeight : mainLayout.height
                ScrollBar.vertical.policy: (mainLayout.height > postDialogItem.basisHeight) ? ScrollBar.AlwaysOn :ScrollBar.AlwaysOff
                clip: true
                Item {
                    implicitWidth: mainLayout.width
                    implicitHeight: mainLayout.height
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.SizeAllCursor
                        drag.target: postDialogPosition
                        drag.minimumX: 0
                        drag.minimumY: 0
                        drag.maximumX: postDialogItem.parentWidth - postDialogPosition.width
                        drag.maximumY: postDialogItem.parentHeight - postDialogPosition.height
                        onPressed: postText.forceActiveFocus()
                    }
                    ColumnLayout {
                        id: mainLayout
                        Frame {
                            id: replyFrame
                            Layout.preferredWidth: postText.width
                            Layout.maximumHeight: 200 * AdjustedValues.ratio
                            visible: postDialogItem.postType === "reply"
                            clip: true
                            ColumnLayout {
                                anchors.fill: parent
                                RowLayout {
                                    AvatarImage {
                                        id: replyAvatarImage
                                        Layout.preferredWidth: AdjustedValues.i16
                                        Layout.preferredHeight: AdjustedValues.i16
                                        source: postDialogItem.replyAvatar
                                    }
                                    Author {
                                        layoutWidth: replyFrame.width - replyFrame.padding * 2 - replyAvatarImage.width - parent.spacing
                                        displayName: postDialogItem.replyDisplayName
                                        handle: postDialogItem.replyHandle
                                        indexedAt: postDialogItem.replyIndexedAt
                                    }
                                }
                                Label {
                                    Layout.preferredWidth: postText.width - replyFrame.padding * 2
                                    wrapMode: Text.Wrap
                                    font.pointSize: AdjustedValues.f8
                                    text: postDialogItem.replyText
                                }
                            }
                        }

                        RowLayout {
                            ComboBox {
                                id: accountCombo
                                Layout.preferredWidth: 250 * AdjustedValues.ratio + AdjustedValues.i24
                                Layout.preferredHeight: implicitHeight * AdjustedValues.ratio
                                enabled: !createRecord.running
                                font.pointSize: AdjustedValues.f10
                                textRole: "handle"
                                valueRole: "did"
                                delegate: ItemDelegate {
                                    id: accountItemDelegate
                                    required property int index
                                    required property string avatar
                                    required property string handle
                                    width: parent.width
                                    height: implicitHeight * AdjustedValues.ratio
                                    font.pointSize: AdjustedValues.f10
                                    onClicked: accountCombo.currentIndex = accountItemDelegate.index
                                    AccountLayout {
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        source: accountItemDelegate.avatar
                                        handle: accountItemDelegate.handle
                                    }
                                }
                                contentItem: AccountLayout {
                                    id: accountAvatarLayout
                                    width: parent.width
                                    height: parent.height
                                    leftMargin: 10
                                    handle: accountCombo.displayText
                                }

                                onCurrentIndexChanged: {
                                    var row = accountCombo.currentIndex
                                    if(row >= 0){
                                        var uuid = postDialogItem.accountModel.item(row, AccountListModel.UuidRole)
                                        accountAvatarLayout.source =
                                        postDialogItem.accountModel.item(row, AccountListModel.AvatarRole)
                                        const langs = postDialogItem.accountModel.item(row, AccountListModel.PostLanguagesRole)
                                        languageSelectionDialog.setSelectedLanguages(langs)
                                        postLanguagesButton.setLanguageText(langs)
                                        selectThreadGateDialog.initialQuoteEnabled = postDialogItem.accountModel.item(row, AccountListModel.PostGateQuoteEnabledRole)
                                        selectThreadGateDialog.initialType = postDialogItem.accountModel.item(row, AccountListModel.ThreadGateTypeRole)
                                        selectThreadGateDialog.initialOptions = postDialogItem.accountModel.item(row, AccountListModel.ThreadGateOptionsRole)
                                        // リプライ制限のダイアログを開かずにポストするときのため選択済みにも設定する
                                        selectThreadGateDialog.selectedQuoteEnabled = selectThreadGateDialog.initialQuoteEnabled
                                        selectThreadGateDialog.selectedType = selectThreadGateDialog.initialType
                                        selectThreadGateDialog.selectedOptions = selectThreadGateDialog.initialOptions
                                        // 入力中にアカウントを切り替えるかもなので選んだ時に設定する
                                        mentionSuggestionView.setAccount(uuid)
                                        // エラーで再表示したときに同じアカウントが選ばれるように保存しておく
                                        postDialogItem.defaultAccountUuid = uuid
                                    }
                                }
                            }
                            Item {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 1
                            }
                            IconButton {
                                id: threadGateButton
                                enabled: !createRecord.running && (postDialogItem.postType !== "reply")
                                iconSource: "../images/thread.png"
                                iconSize: AdjustedValues.i18
                                flat: true
                                foreground: (selectThreadGateDialog.selectedType !== "everybody" || !selectThreadGateDialog.selectedQuoteEnabled)
                                            ? Material.accent : Material.foreground
                                onClicked: {
                                    var row = accountCombo.currentIndex;
                                    if(selectThreadGateDialog.account.set(postDialogItem.accountModel,
                                                                          postDialogItem.accountModel.item(row, AccountListModel.UuidRole))){
                                        selectThreadGateDialog.initialQuoteEnabled = selectThreadGateDialog.selectedQuoteEnabled
                                        selectThreadGateDialog.initialType = selectThreadGateDialog.selectedType
                                        selectThreadGateDialog.initialOptions = selectThreadGateDialog.selectedOptions
                                        selectThreadGateDialog.open()
                                    }
                                }
                            }

                            IconButton {
                                id: postLanguagesButton
                                enabled: !createRecord.running
                                iconSource: "../images/language.png"
                                iconSize: AdjustedValues.i18
                                flat: true
                                onClicked: {
                                    languageSelectionDialog.setSelectedLanguages(
                                        postDialogItem.accountModel.item(accountCombo.currentIndex, AccountListModel.PostLanguagesRole)
                                        )
                                    languageSelectionDialog.open()
                                }

                                function setLanguageText(post_langs){
                                    var langs = languageListModel.convertLanguageNames(post_langs)
                                    var lang_str = ""
                                    for(var i=0;i<langs.length;i++){
                                        if(lang_str.length > 0){
                                            lang_str += ", "
                                        }
                                        lang_str += langs[i]
                                    }
                                    if(lang_str.length > 13){
                                        lang_str = lang_str.substring(0, 10) + "..."
                                    }
                                    iconText = lang_str
                                }
                            }
                        }

                        ScrollView {
                            z: 99   // MentionSuggetionViewを最前に表示するため
                            Layout.preferredWidth: 500 * AdjustedValues.ratio
                            Layout.preferredHeight: 150 * AdjustedValues.ratio
                            TextArea {
                                id: postText
                                verticalAlignment: TextInput.AlignTop
                                enabled: !createRecord.running
                                wrapMode: TextInput.WordWrap
                                selectByMouse: true
                                font.pointSize: AdjustedValues.f10
                                property int realTextLength: systemTool.countText(text)
                                onTextChanged: mentionSuggestionView.reload(getText(0, cursorPosition))
                                Keys.onPressed: (event) => {
                                    if(mentionSuggestionView.visible){
                                        // console.log("Key(v):" + event.key)
                                        if(event.key === Qt.Key_Up){
                                            mentionSuggestionView.up()
                                            event.accepted = true
                                        }else if(event.key === Qt.Key_Down){
                                            mentionSuggestionView.down()
                                            event.accepted = true
                                        }else if(event.key === Qt.Key_Enter ||
                                                 event.key === Qt.Key_Return){
                                            mentionSuggestionView.accept()
                                            event.accepted = true
                                        }else if(event.key === Qt.Key_Escape){
                                            mentionSuggestionView.clear()
                                        }
                                    }else{
                                        // console.log("Key(n):" + event.key)
                                        if(event.key === Qt.Key_Space && (event.modifiers & Qt.ControlModifier)){
                                            mentionSuggestionView.reload(getText(0, cursorPosition))
                                            event.accepted = true
                                        }
                                    }
                                }
                                MentionSuggestionView {
                                    id: mentionSuggestionView
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    onVisibleChanged: {
                                        var rect = postText.positionToRectangle(postText.cursorPosition)
                                        y = rect.y + rect.height + 2
                                    }
                                    onSelected: (handle) => {
                                        var after = replaceText(postText.text, postText.cursorPosition, handle)
                                        if(after !== postText.text){
                                            postText.text = after
                                            postText.cursorPosition = postText.text.length
                                        }
                                    }
                                }
                            }
                        }

                        RowLayout {
                            Layout.preferredWidth: postText.width
                            visible: embedImageListModel.count === 0 &&
                                     addPollDialog.appliedOptions.length === 0
                            ScrollView {
                                Layout.fillWidth: true
                                clip: true
                                TextArea {
                                    id: addingExternalLinkUrlText
                                    selectByMouse: true
                                    font.pointSize: AdjustedValues.f10
                                    placeholderText: (postDialogItem.quoteValid === false) ?
                                                         qsTr("Link card URL, Custom feed URL, List URL, Post URL") :
                                                         qsTr("Link card URL")
                                }
                            }
                            IconButton {
                                id: externalLinkButton
                                iconSource: "../images/add.png"
                                enabled: addingExternalLinkUrlText.text.length > 0 &&
                                         !externalLink.running &&
                                         !feedGeneratorLink.running &&
                                         !postLink.running &&
                                         !listLink.running &&
                                         !createRecord.running
                                onClicked: {
                                    var uri = addingExternalLinkUrlText.text
                                    var row = accountCombo.currentIndex
                                    if(feedGeneratorLink.checkUri(uri, "feed") && !postDialogItem.quoteValid){
                                        feedGeneratorLink.setAccount(postDialogItem.accountModel.item(row, AccountListModel.UuidRole))
                                        feedGeneratorLink.getFeedGenerator(uri)
                                    }else if(listLink.checkUri(uri, "lists") && !postDialogItem.quoteValid){
                                        listLink.setAccount(postDialogItem.accountModel.item(row, AccountListModel.UuidRole))
                                        listLink.getList(uri)
                                    }else if(postLink.checkUri(uri, "post") && !postDialogItem.quoteValid && postDialogItem.postType !== "quote"){
                                        postLink.setAccount(postDialogItem.accountModel.item(row, AccountListModel.UuidRole))
                                        postLink.getPost(uri)
                                    }else{
                                        externalLink.getExternalLink(uri)

                                    }
                                }
                                BusyIndicator {
                                    anchors.fill: parent
                                    anchors.margins: 3
                                    visible: externalLink.running ||
                                             feedGeneratorLink.running ||
                                             postLink.running ||
                                             listLink.running
                                }
                                states: [
                                    State {
                                        when: externalLink.valid ||
                                              feedGeneratorLink.valid ||
                                              listLink.valid ||
                                              (postLink.valid && postDialogItem.postType !== "quote")
                                        PropertyChanges {
                                            target: externalLinkButton
                                            iconSource: "../images/delete.png"
                                            onClicked: {
                                                externalLink.clear()
                                                feedGeneratorLink.clear()
                                                listLink.clear()
                                                postLink.clear()
                                                if(postDialogItem.postType !== "quote"){
                                                    postDialogItem.quoteCid = ""
                                                    postDialogItem.quoteUri = ""
                                                    postDialogItem.quoteAvatar = ""
                                                    postDialogItem.quoteDisplayName = ""
                                                    postDialogItem.quoteHandle = ""
                                                    postDialogItem.quoteIndexedAt = ""
                                                    postDialogItem.quoteText = ""
                                                }
                                            }
                                        }
                                    }
                                ]
                            }
                        }
                        ExternalLinkCard {
                            Layout.preferredWidth: postText.width
                            // Layout.maximumHeight: 280 * AdjustedValues.ratio
                            visible: externalLink.valid

                            thumbImage.source: externalLink.thumbLocal
                            uriLabel.text: externalLink.uri
                            titleLabel.text: externalLink.title
                            descriptionLabel.text: externalLink.description
                        }
                        FeedGeneratorLinkCard {
                            Layout.preferredWidth: postText.width
                            visible: feedGeneratorLink.valid

                            avatarImage.source: feedGeneratorLink.avatar
                            displayNameLabel.text: feedGeneratorLink.displayName
                            creatorHandleLabel.text: feedGeneratorLink.creatorHandle
                            likeCountLabel.text: feedGeneratorLink.likeCount
                        }
                        ListLinkCard {
                            Layout.preferredWidth: postText.width
                            visible: listLink.valid
                            avatarImage.source: listLink.avatar
                            displayNameLabel.text: listLink.displayName
                            creatorHandleLabel.text: listLink.creatorHandle
                            descriptionLabel.text: listLink.description
                        }
                        PollOptions {
                            Layout.preferredWidth: postText.width
                            visible: addPollDialog.appliedOptions.length > 0
                            model: addPollDialog.appliedOptions
                        }

                        ScrollView {
                            Layout.preferredWidth: postText.width
                            Layout.preferredHeight: 102 * AdjustedValues.ratio + ScrollBar.horizontal.height + 1
                            ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
                            visible: embedImageListModel.count > 0
                            enabled: !createRecord.running
                            clip: true
                            RowLayout {
                                spacing: 4 * AdjustedValues.ratio
                                Repeater {
                                    model: EmbedImageListModel {
                                        id: embedImageListModel
                                        property int adjustPostLength: count > 4 ? 5 + (Math.ceil(count / 4) + "").length : 0
                                    }
                                    delegate: ImageWithIndicator {
                                        id: additionalImages
                                        required property int index
                                        required property string uri
                                        required property string alt
                                        required property string number
                                        Layout.preferredWidth: 102 * AdjustedValues.ratio
                                        Layout.preferredHeight: 102 * AdjustedValues.ratio
                                        fillMode: Image.PreserveAspectCrop
                                        source: additionalImages.uri
                                        TagLabel {
                                            anchors.left: parent.left
                                            anchors.bottom: parent.bottom
                                            anchors.margins: 3
                                            visible: additionalImages.alt.length > 0
                                            source: ""
                                            fontPointSize: AdjustedValues.f8
                                            text: "Alt"
                                        }
                                        TagLabel {
                                            anchors.right: parent.right
                                            anchors.bottom: parent.bottom
                                            anchors.margins: 3
                                            visible: additionalImages.number.length > 0
                                            source: ""
                                            fontPointSize: AdjustedValues.f8
                                            text: additionalImages.number
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                altEditDialog.editingIndex = additionalImages.index
                                                altEditDialog.embedImage = additionalImages.uri
                                                altEditDialog.embedAlt = additionalImages.alt
                                                altEditDialog.open()
                                            }
                                        }
                                        IconButton {
                                            enabled: !createRecord.running
                                            width: AdjustedValues.b24
                                            height: AdjustedValues.b24
                                            anchors.top: parent.top
                                            anchors.right: parent.right
                                            anchors.margins: 5
                                            iconSource: "../images/delete.png"
                                            onClicked: embedImageListModel.remove(additionalImages.index)
                                        }
                                    }
                                }
                            }
                        }

                        Frame {
                            id: quoteFrame
                            Layout.preferredWidth: postText.width
                            Layout.maximumHeight: 200 * AdjustedValues.ratio
                            visible: postDialogItem.quoteValid
                            clip: true
                            ColumnLayout {
                                Layout.preferredWidth: postText.width
                                RowLayout {
                                    AvatarImage {
                                        id: quoteAvatarImage
                                        Layout.preferredWidth: AdjustedValues.i16
                                        Layout.preferredHeight: AdjustedValues.i16
                                        source: postDialogItem.quoteAvatar
                                    }
                                    Author {
                                        layoutWidth: postText.width - quoteFrame.padding * 2 - quoteAvatarImage.width - parent.spacing
                                        displayName: postDialogItem.quoteDisplayName
                                        handle: postDialogItem.quoteHandle
                                        indexedAt: postDialogItem.quoteIndexedAt
                                    }
                                }
                                Label {
                                    Layout.preferredWidth: postText.width - quoteFrame.padding * 2
                                    wrapMode: Text.Wrap
                                    font.pointSize: AdjustedValues.f8
                                    text: postDialogItem.quoteText
                                }
                            }
                        }

                        RowLayout {
                            spacing: 0
                            Button {
                                enabled: !createRecord.running
                                flat: true
                                font.pointSize: AdjustedValues.f10
                                text: qsTr("Cancel")
                                onClicked: {
                                    if(postButton.enabled){
                                        // 下書き保存
                                        draftConfirmationDialog.show("normal", qsTr("Confirm"), qsTr("Save the draft?"))
                                    }else{
                                        postDialogItem.close()
                                    }
                                }
                            }
                            Item {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 1
                            }
                            IconButton {
                                id: selfLabelsButton
                                enabled: !createRecord.running
                                iconSource: "../images/labeling.png"
                                iconSize: AdjustedValues.i18
                                flat: true
                                foreground: value.length > 0 ? Material.accent : Material.foreground
                                onClicked: selfLabelPopup.popup()
                                property string value: ""
                                SelfLabelPopup {
                                    id: selfLabelPopup
                                    onTriggered: (value, text) => {
                                        if(value.length > 0){
                                            selfLabelsButton.value = value
                                            selfLabelsButton.iconText = text
                                        }else{
                                            selfLabelsButton.value = ""
                                            selfLabelsButton.iconText = ""
                                        }
                                    }
                                    onClosed: postText.forceActiveFocus()
                                }
                            }
                            IconButton {
                                enabled: addPollDialog.appliedOptions.length === 0 &&
                                         !createRecord.running &&
                                         !externalLink.valid &&
                                         !feedGeneratorLink.valid &&
                                         !listLink.valid &&
                                         !embedImageListModel.running
                                iconSource: "../images/add_image.png"
                                iconSize: AdjustedValues.i18
                                flat: true
                                onClicked: {
                                    if(fileDialog.prevFolder.length > 0){
                                        fileDialog.folder = fileDialog.prevFolder
                                    }
                                    fileDialog.open()
                                }
                            }
                            IconButton {
                                enabled: !createRecord.running &&
                                         !externalLink.valid &&
                                         !feedGeneratorLink.valid &&
                                         !listLink.valid &&
                                         !embedImageListModel.count > 0 &&
                                         !embedImageListModel.running
                                iconSource: "../images/chart.png"
                                iconSize: AdjustedValues.i18
                                foreground: addPollDialog.appliedOptions.length > 0 ? Material.accent : Material.foreground
                                flat: true
                                onClicked: {
                                    addPollDialog.open()
                                }
                            }

                            Label {
                                Layout.leftMargin: 5
                                Layout.alignment: Qt.AlignVCenter
                                font.pointSize: AdjustedValues.f8
                                text: 300 - embedImageListModel.adjustPostLength - postText.realTextLength
                            }
                            ProgressCircle {
                                Layout.leftMargin: 5
                                Layout.preferredWidth: AdjustedValues.i24
                                Layout.preferredHeight: AdjustedValues.i24
                                Layout.alignment: Qt.AlignVCenter
                                from: 0
                                to: 300 - embedImageListModel.adjustPostLength
                                value: postText.realTextLength
                            }
                            Button {
                                id: postButton
                                Layout.alignment: Qt.AlignRight
                                enabled: postText.text.length > 0 &&
                                         postText.realTextLength <= (300 - embedImageListModel.adjustPostLength) &&
                                         !createRecord.running &&
                                         !externalLink.running &&
                                         !feedGeneratorLink.running &&
                                         !listLink.running
                                font.pointSize: AdjustedValues.f10
                                text: qsTr("Post")
                                onClicked: {
                                    postDialog.close()

                                    var row = accountCombo.currentIndex;
                                    createRecord.setAccount(postDialogItem.accountModel.item(row, AccountListModel.UuidRole))
                                    createRecord.clear()
                                    createRecord.setText(postText.text)
                                    createRecord.setPostLanguages(languageSelectionDialog.selectedLanguages)
                                    if(postDialogItem.postType !== "reply"){
                                        // replyのときは制限の設定はできない
                                        createRecord.setThreadGate(selectThreadGateDialog.selectedType, selectThreadGateDialog.selectedOptions)
                                        createRecord.setPostGate(selectThreadGateDialog.selectedQuoteEnabled, [])
                                    }
                                    if(postDialogItem.postType === "reply"){
                                        createRecord.setReply(postDialogItem.replyCid, postDialogItem.replyUri, postDialogItem.replyRootCid, postDialogItem.replyRootUri)
                                    }
                                    if(postDialogItem.quoteValid){
                                        createRecord.setQuote(postDialogItem.quoteCid, postDialogItem.quoteUri)
                                    }
                                    if(selfLabelsButton.value.length > 0){
                                        createRecord.setSelfLabels([selfLabelsButton.value])
                                    }
                                    if(externalLink.valid){
                                        createRecord.setExternalLink(externalLink.uri, externalLink.title, externalLink.description, externalLink.thumbLocal)
                                        createRecord.postWithImages()
                                    }else if(feedGeneratorLink.valid){
                                        createRecord.setFeedGeneratorLink(feedGeneratorLink.uri, feedGeneratorLink.cid)
                                        createRecord.post()
                                    }else if(listLink.valid){
                                        createRecord.setFeedGeneratorLink(listLink.uri, listLink.cid)
                                        createRecord.post()
                                    }else if(embedImageListModel.count > 0){
                                        createRecord.setImages(embedImageListModel.uris(), embedImageListModel.alts())
                                        createRecord.postWithImages()
                                    }else if(addPollDialog.appliedOptions.length > 0){
                                        createRecord.setPoll(addPollDialog.appliedOptions, addPollDialog.appliedDuration)
                                        createRecord.postWithPoll()
                                    }else{
                                        createRecord.post()
                                    }
                                }
                                BusyIndicator {
                                    anchors.fill: parent
                                    anchors.margins: 3
                                    visible: createRecord.running
                                }
                            }
                        }
                    }
                    DragAndDropArea {
                        anchors.fill: parent
                        anchors.margins: -5
                        onDropped: (urls) => embedImageListModel.append(urls)
                    }
                }
            }

            P.FileDialog {
                id: fileDialog
                title: qsTr("Select contents")
                visible: false
                fileMode : P.FileDialog.OpenFiles
                nameFilters: ["Image files (*.jpg *.jpeg *.png *.gif)"
                    , "All files (*)"]
                onAccepted: {
                    //選択されたファイルをすべて追加
                    prevFolder = folder

                    var new_files = []
                    for(var i=0; i<files.length; i++){
                        new_files.push(files[i])
                    }
                    embedImageListModel.append(new_files)
                }
                property string prevFolder
            }

            LanguageSelectionDialog {
                id: languageSelectionDialog
                onAccepted: {
                    postDialogItem.accountModel.update(accountCombo.currentIndex, AccountListModel.PostLanguagesRole, selectedLanguages)
                    postLanguagesButton.setLanguageText(selectedLanguages)
                }
            }

            AltEditDialog {
                id: altEditDialog
                property int editingIndex: -1
                onAccepted: embedImageListModel.updateAlt(editingIndex, altEditDialog.embedAlt)
            }

            SelectThreadGateDialog {
                id: selectThreadGateDialog

                onAccepted: {

                }
                onClosed: postText.forceActiveFocus()
            }

            AddPollDialog {
                id: addPollDialog
                onAccepted: {

                }
            }

            MessageDialog {
                id: draftConfirmationDialog
                useCancel: true
                acceptButtonText: qsTr("Yes")
                rejectButtonText: qsTr("No")
                onRejected: postDialogItem.close()
                onAccepted: {
                    postDialog.close()

                    var row = accountCombo.currentIndex;
                    draftOperator.setAccount(postDialogItem.accountModel.item(row, AccountListModel.UuidRole))
                    draftOperator.clear()
                    draftOperator.setText(postText.text)
                    draftOperator.setPostLanguages(languageSelectionDialog.selectedLanguages)
                    if(postDialogItem.postType !== "reply"){
                        // replyのときは制限の設定はできない
                        draftOperator.setThreadGate(selectThreadGateDialog.selectedType, selectThreadGateDialog.selectedOptions)
                        draftOperator.setPostGate(selectThreadGateDialog.selectedQuoteEnabled, [])
                    }
                    if(postDialogItem.quoteValid){
                        draftOperator.setQuote(postDialogItem.quoteCid, postDialogItem.quoteUri)
                    }
                    if(selfLabelsButton.value.length > 0){
                        draftOperator.setSelfLabels([selfLabelsButton.value])
                    }
                    draftOperator.setExternalLink(addingExternalLinkUrlText.text)
                    if(embedImageListModel.count > 0){
                        draftOperator.setImages(embedImageListModel.uris(), embedImageListModel.alts())
                    }
                    draftOperator.createDraft()
                }
            }
        }
    }
}
