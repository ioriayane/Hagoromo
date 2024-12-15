import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import Qt.labs.platform 1.1 as P

import tech.relog.hagoromo.recordoperator 1.0
import tech.relog.hagoromo.accountlistmodel 1.0
import tech.relog.hagoromo.languagelistmodel 1.0
import tech.relog.hagoromo.externallink 1.0
import tech.relog.hagoromo.feedgeneratorlink 1.0
import tech.relog.hagoromo.embedimagelistmodel 1.0
import tech.relog.hagoromo.listlink 1.0
import tech.relog.hagoromo.postlink 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../parts"

Dialog {
    id: postDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - scrollView.implicitHeight - postDialog.topPadding - postDialog.bottomPadding) * 0.5
    closePolicy: Popup.NoAutoClose
    topPadding: 20
    bottomPadding: 20
    rightPadding: 0
    property real basisHeight: parent.height * 0.9 - postDialog.topPadding - postDialog.bottomPadding

    property int parentWidth: parent.width
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

    function openWithFiles(urls){
        if(embedImageListModel.append(urls)){
            postDialog.open()
        }
    }

    onOpened: {
        var i = accountModel.indexAt(defaultAccountUuid)
        accountCombo.currentIndex = -1
        if(i >= 0){
            accountCombo.currentIndex = i
        } else {
            accountCombo.currentIndex = accountModel.getMainAccountIndex()
        }
        postText.forceActiveFocus()
    }
    onClosed: {
        defaultAccountUuid = ""
        postType = "normal"
        replyCid = ""
        replyUri = ""
        replyAvatar = ""
        replyDisplayName = ""
        replyHandle = ""
        replyIndexedAt = ""
        replyText = ""
        quoteCid = ""
        quoteUri = ""
        quoteAvatar = ""
        quoteDisplayName = ""
        quoteHandle = ""
        quoteIndexedAt = ""
        quoteText = ""
        postLanguagesButton.text = ""
        selfLabelsButton.value = ""
        selfLabelsButton.iconText = ""

        postText.clear()
        embedImageListModel.clear()
        externalLink.clear()
        feedGeneratorLink.clear()
        listLink.clear()
        postLink.clear()
        addingExternalLinkUrlText.text = ""
        selectThreadGateDialog.clear()
    }

    Shortcut {  // Post
        enabled: postDialog.visible && postButton.enabled && postText.focus
        sequence: "Ctrl+Return"
        onActivated: postButton.clicked()
    }
    Shortcut {  // Close
        // DialogのclosePolicyでEscで閉じられるけど、そのうち編集中の確認ダイアログを
        // 入れたいので別でイベント処理をする。onClosedで閉じるをキャンセルできなさそうなので。
        enabled: postDialog.visible && ! postButton.enabled
        sequence: "Esc"
        onActivated: postDialog.close()
    }

    SystemTool {
        id: systemTool
    }
    RecordOperator {
        id: createRecord
        onFinished: (success) => {
                        if(success){
                            postText.clear()
                            postDialog.close()
                        }
                    }
        onErrorOccured: (code, message) => {
                            var row = accountCombo.currentIndex;
                            postDialog.errorOccured(postDialog.accountModel.item(row, AccountListModel.UuidRole), code, message)
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
            if(valid){
                quoteCid = postLink.cid
                quoteUri = postLink.uri
                quoteAvatar = postLink.avatar
                quoteDisplayName = postLink.displayName
                quoteHandle = postLink.creatorHandle
                quoteIndexedAt = postLink.indexedAt
                quoteText = postLink.text
            }
        }
    }

    ScrollView {
        id: scrollView
        implicitWidth: mainLayout.width + postDialog.leftPadding
        implicitHeight: (mainLayout.height > basisHeight) ? basisHeight : mainLayout.height
        ScrollBar.vertical.policy: (mainLayout.height > basisHeight) ? ScrollBar.AlwaysOn :ScrollBar.AlwaysOff
        clip: true
        Item {
            implicitWidth: mainLayout.width
            implicitHeight: mainLayout.height
            ColumnLayout {
                id: mainLayout
                Frame {
                    id: replyFrame
                    Layout.preferredWidth: postText.width
                    Layout.maximumHeight: 200 * AdjustedValues.ratio
                    visible: postType === "reply"
                    clip: true
                    ColumnLayout {
                        anchors.fill: parent
                        RowLayout {
                            AvatarImage {
                                id: replyAvatarImage
                                Layout.preferredWidth: AdjustedValues.i16
                                Layout.preferredHeight: AdjustedValues.i16
                                source: replyAvatar
                            }
                            Author {
                                layoutWidth: replyFrame.width - replyFrame.padding * 2 - replyAvatarImage.width - parent.spacing
                                displayName: replyDisplayName
                                handle: replyHandle
                                indexedAt: replyIndexedAt
                            }
                        }
                        Label {
                            Layout.preferredWidth: postText.width - replyFrame.padding * 2
                            wrapMode: Text.WrapAnywhere
                            font.pointSize: AdjustedValues.f8
                            text: replyText
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
                            width: parent.width
                            height: implicitHeight * AdjustedValues.ratio
                            font.pointSize: AdjustedValues.f10
                            onClicked: accountCombo.currentIndex = model.index
                            AccountLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                source: model.avatar
                                handle: model.handle
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
                                accountAvatarLayout.source =
                                        postDialog.accountModel.item(row, AccountListModel.AvatarRole)
                                postLanguagesButton.setLanguageText(
                                            postDialog.accountModel.item(row, AccountListModel.PostLanguagesRole)
                                            )
                                selectThreadGateDialog.initialQuoteEnabled = postDialog.accountModel.item(row, AccountListModel.PostGateQuoteEnabledRole)
                                selectThreadGateDialog.initialType = postDialog.accountModel.item(row, AccountListModel.ThreadGateTypeRole)
                                selectThreadGateDialog.initialOptions = postDialog.accountModel.item(row, AccountListModel.ThreadGateOptionsRole)
                                // リプライ制限のダイアログを開かずにポストするときのため選択済みにも設定する
                                selectThreadGateDialog.selectedQuoteEnabled = selectThreadGateDialog.initialQuoteEnabled
                                selectThreadGateDialog.selectedType = selectThreadGateDialog.initialType
                                selectThreadGateDialog.selectedOptions = selectThreadGateDialog.initialOptions
                                // 入力中にアカウントを切り替えるかもなので選んだ時に設定する
                                mentionSuggestionView.setAccount(postDialog.accountModel.item(row, AccountListModel.UuidRole))
                            }
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                    }
                    IconButton {
                        id: threadGateButton
                        enabled: !createRecord.running && (postType !== "reply")
                        iconSource: "../images/thread.png"
                        iconSize: AdjustedValues.i18
                        flat: true
                        foreground: (selectThreadGateDialog.selectedType !== "everybody" || !selectThreadGateDialog.selectedQuoteEnabled)
                                    ? Material.accent : Material.foreground
                        onClicked: {
                            var row = accountCombo.currentIndex;
                            selectThreadGateDialog.account.service = postDialog.accountModel.item(row, AccountListModel.ServiceRole)
                            selectThreadGateDialog.account.did = postDialog.accountModel.item(row, AccountListModel.DidRole)
                            selectThreadGateDialog.account.handle = postDialog.accountModel.item(row, AccountListModel.HandleRole)
                            selectThreadGateDialog.account.email = postDialog.accountModel.item(row, AccountListModel.EmailRole)
                            selectThreadGateDialog.account.accessJwt = postDialog.accountModel.item(row, AccountListModel.AccessJwtRole)
                            selectThreadGateDialog.account.refreshJwt = postDialog.accountModel.item(row, AccountListModel.RefreshJwtRole)
                            selectThreadGateDialog.initialQuoteEnabled = selectThreadGateDialog.selectedQuoteEnabled
                            selectThreadGateDialog.initialType = selectThreadGateDialog.selectedType
                            selectThreadGateDialog.initialOptions = selectThreadGateDialog.selectedOptions
                            selectThreadGateDialog.open()
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
                                        postDialog.accountModel.item(accountCombo.currentIndex, AccountListModel.PostLanguagesRole)
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
                                                console.log("Key(n):" + event.key)
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
                    visible: embedImageListModel.count === 0
                    ScrollView {
                        Layout.fillWidth: true
                        clip: true
                        TextArea {
                            id: addingExternalLinkUrlText
                            selectByMouse: true
                            font.pointSize: AdjustedValues.f10
                            placeholderText: (quoteValid === false) ?
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
                                 !listLink.running &&
                                 !createRecord.running
                        onClicked: {
                            var uri = addingExternalLinkUrlText.text
                            var row = accountCombo.currentIndex
                            if(feedGeneratorLink.checkUri(uri, "feed") && !quoteValid){
                                feedGeneratorLink.setAccount(postDialog.accountModel.item(row, AccountListModel.UuidRole))
                                feedGeneratorLink.getFeedGenerator(uri)
                            }else if(listLink.checkUri(uri, "lists") && !quoteValid){
                                listLink.setAccount(postDialog.accountModel.item(row, AccountListModel.UuidRole))
                                listLink.getList(uri)
                            }else if(postLink.checkUri(uri, "post") && !quoteValid && postType !== "quote"){
                                postLink.setAccount(postDialog.accountModel.item(row, AccountListModel.UuidRole))
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
                                     listLink.running
                        }
                        states: [
                            State {
                                when: externalLink.valid ||
                                      feedGeneratorLink.valid ||
                                      listLink.valid ||
                                      (postLink.valid && postType !== "quote")
                                PropertyChanges {
                                    target: externalLinkButton
                                    iconSource: "../images/delete.png"
                                    onClicked: {
                                        externalLink.clear()
                                        feedGeneratorLink.clear()
                                        listLink.clear()
                                        postLink.clear()
                                        if(postType !== "quote"){
                                            quoteCid = ""
                                            quoteUri = ""
                                            quoteAvatar = ""
                                            quoteDisplayName = ""
                                            quoteHandle = ""
                                            quoteIndexedAt = ""
                                            quoteText = ""
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
                                Layout.preferredWidth: 102 * AdjustedValues.ratio
                                Layout.preferredHeight: 102 * AdjustedValues.ratio
                                fillMode: Image.PreserveAspectCrop
                                source: model.uri
                                TagLabel {
                                    anchors.left: parent.left
                                    anchors.bottom: parent.bottom
                                    anchors.margins: 3
                                    visible: model.alt.length > 0
                                    source: ""
                                    fontPointSize: AdjustedValues.f8
                                    text: "Alt"
                                }
                                TagLabel {
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom
                                    anchors.margins: 3
                                    visible: model.number.length > 0
                                    source: ""
                                    fontPointSize: AdjustedValues.f8
                                    text: model.number
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        altEditDialog.editingIndex = model.index
                                        altEditDialog.embedImage = model.uri
                                        altEditDialog.embedAlt = model.alt
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
                                    onClicked: embedImageListModel.remove(model.index)
                                }
                            }
                        }
                    }
                }

                Frame {
                    id: quoteFrame
                    Layout.preferredWidth: postText.width
                    Layout.maximumHeight: 200 * AdjustedValues.ratio
                    visible: quoteValid
                    clip: true
                    ColumnLayout {
                        Layout.preferredWidth: postText.width
                        RowLayout {
                            AvatarImage {
                                id: quoteAvatarImage
                                Layout.preferredWidth: AdjustedValues.i16
                                Layout.preferredHeight: AdjustedValues.i16
                                source: quoteAvatar
                            }
                            Author {
                                layoutWidth: postText.width - quoteFrame.padding * 2 - quoteAvatarImage.width - parent.spacing
                                displayName: quoteDisplayName
                                handle: quoteHandle
                                indexedAt: quoteIndexedAt
                            }
                        }
                        Label {
                            Layout.preferredWidth: postText.width - quoteFrame.padding * 2
                            wrapMode: Text.WrapAnywhere
                            font.pointSize: AdjustedValues.f8
                            text: quoteText
                        }
                    }
                }

                Label {
                    Layout.preferredWidth: postText.width
                    Layout.leftMargin: 10
                    Layout.topMargin: 2
                    Layout.bottomMargin: 2
                    font.pointSize: AdjustedValues.f8
                    text: createRecord.progressMessage
                    visible: createRecord.running && createRecord.progressMessage.length > 0
                    color: Material.theme === Material.Dark ? Material.foreground : "white"
                    Rectangle {
                        anchors.fill: parent
                        anchors.leftMargin: -10
                        anchors.topMargin: -2
                        anchors.bottomMargin: -2
                        z: -1
                        radius: height / 2
                        color: Material.color(Material.Indigo)
                    }
                }

                RowLayout {
                    spacing: 0
                    Button {
                        enabled: !createRecord.running
                        flat: true
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Cancel")
                        onClicked: postDialog.close()
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
                        enabled: !createRecord.running &&
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
                            var row = accountCombo.currentIndex;
                            createRecord.setAccount(postDialog.accountModel.item(row, AccountListModel.UuidRole))
                            createRecord.clear()
                            createRecord.setText(postText.text)
                            createRecord.setPostLanguages(postDialog.accountModel.item(row, AccountListModel.PostLanguagesRole))
                            if(postType !== "reply"){
                                // replyのときは制限の設定はできない
                                createRecord.setThreadGate(selectThreadGateDialog.selectedType, selectThreadGateDialog.selectedOptions)
                                createRecord.setPostGate(selectThreadGateDialog.selectedQuoteEnabled, [])
                            }
                            if(postType === "reply"){
                                createRecord.setReply(replyCid, replyUri, replyRootCid, replyRootUri)
                            }
                            if(quoteValid){
                                createRecord.setQuote(quoteCid, quoteUri)
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
            postDialog.accountModel.update(accountCombo.currentIndex, AccountListModel.PostLanguagesRole, selectedLanguages)
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
}
