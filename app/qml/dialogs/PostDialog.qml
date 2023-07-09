import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1 as P

import tech.relog.hagoromo.recordoperator 1.0
import tech.relog.hagoromo.accountlistmodel 1.0
import tech.relog.hagoromo.languagelistmodel 1.0

import "../controls"
import "../parts"

Dialog {
    id: postDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    closePolicy: Popup.NoAutoClose

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

    property alias postText: postText

    onOpened: {
        var i = accountModel.indexAt(defaultAccountUuid)
        accountCombo.currentIndex = -1
        if(i >= 0){
            accountCombo.currentIndex = i
        } else {
            accountCombo.currentIndex = 0
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
        postLanguagesButton.text = ""

        postText.clear()
        embedImagePreview.embedImages = ""
    }

    Shortcut {  // Post
        enabled: postButton.enabled && postText.focus
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


    RecordOperator {
        id: createRecord
        onFinished: (success) => {
                        if(success){
                            postText.clear()
                            postDialog.close()
                        }
                    }
    }
    LanguageListModel {
        id: languageListModel
    }

    ColumnLayout {

        Frame {
            id: replyFrame
            Layout.preferredWidth: postText.width
            visible: postType === "reply"
            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    Image {
                        id: replyAvatarImage
                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 16
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
                    font.pointSize: 8
                    text: replyText
                }
            }
        }

        RowLayout {
            AvatarImage {
                id: accountAvatarImage
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                //                source:
            }

            ComboBox {
                id: accountCombo
                Layout.preferredWidth: 200
                enabled: !createRecord.running
                textRole: "handle"
                valueRole: "did"
                delegate: ItemDelegate {
                    text: model.handle
                    width: parent.width
                    onClicked: accountCombo.currentIndex = model.index
                }
                onCurrentIndexChanged: {
                    if(accountCombo.currentIndex >= 0){
                        accountAvatarImage.source =
                                postDialog.accountModel.item(accountCombo.currentIndex, AccountListModel.AvatarRole)
                        postLanguagesButton.setLanguageText(
                                    postDialog.accountModel.item(accountCombo.currentIndex, AccountListModel.PostLanguagesRole)
                                    )
                    }
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            IconButton {
                id: postLanguagesButton
                iconSource: "../images/language.png"
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
            Layout.preferredWidth: 400
            Layout.preferredHeight: 100
            TextArea {
                id: postText
                verticalAlignment: TextInput.AlignTop
                enabled: !createRecord.running
                wrapMode: TextInput.WordWrap
                selectByMouse: true
            }
        }


        RowLayout {
            visible: embedImagePreview.embedImages.length > 0
            spacing: 4
            Repeater {
                id: embedImagePreview
                property string embedImages: ""
                model: embedImagePreview.embedImages.split("\n")
                delegate: ImageWithIndicator {
                    Layout.preferredWidth: 97
                    Layout.preferredHeight: 97
                    fillMode: Image.PreserveAspectCrop
                    source: modelData
                    IconButton {
                        enabled: !createRecord.running
                        width: 24
                        height: 24
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: 5
                        iconSource: "../images/delete.png"
                        onClicked: {
                            var images = embedImagePreview.embedImages.split("\n")
                            var new_images = ""
                            for(var i=0; i<images.length; i++){
                                if(images[i] === modelData){
                                    continue;
                                }
                                if(new_images.length > 0){
                                    new_images += "\n"
                                }
                                new_images += images[i]
                            }
                            embedImagePreview.embedImages = new_images
                        }
                    }
                }
            }
        }

        Frame {
            id: quoteFrame
            Layout.preferredWidth: postText.width
            visible: postType === "quote"
            ColumnLayout {
                Layout.preferredWidth: postText.width
                RowLayout {
                    Image {
                        id: quoteAvatarImage
                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 16
                        source: replyAvatar
                    }
                    Author {
                        layoutWidth: postText.width - quoteFrame.padding * 2 - quoteAvatarImage.width - parent.spacing
                        displayName: replyDisplayName
                        handle: replyHandle
                        indexedAt: replyIndexedAt
                    }
                }
                Label {
                    Layout.preferredWidth: postText.width - quoteFrame.padding * 2
                    wrapMode: Text.WrapAnywhere
                    font.pointSize: 8
                    text: replyText
                }
            }
        }

        RowLayout {
            //            Layout.alignment: Qt.AlignRight
            Button {
                enabled: !createRecord.running
                flat: true
                text: qsTr("Cancel")
                onClicked: postDialog.close()
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            IconButton {
                enabled: !createRecord.running
                iconSource: "../images/add_image.png"
                flat: true
                onClicked: {
                    if(fileDialog.prevFolder.length > 0){
                        fileDialog.folder = fileDialog.prevFolder
                    }
                    fileDialog.open()
                }
            }

            Label {
                Layout.alignment: Qt.AlignVCenter
                font.pointSize: 8
                text: 300 - postText.text.length
            }
            ProgressCircle {
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                Layout.alignment: Qt.AlignVCenter
                from: 0
                to: 300
                value: postText.text.length
            }
            Button {
                id: postButton
                Layout.alignment: Qt.AlignRight
                enabled: postText.text.length > 0 && !createRecord.running
                text: qsTr("Post")
                onClicked: {
                    var row = accountCombo.currentIndex;
                    createRecord.setAccount(postDialog.accountModel.item(row, AccountListModel.ServiceRole),
                                            postDialog.accountModel.item(row, AccountListModel.DidRole),
                                            postDialog.accountModel.item(row, AccountListModel.HandleRole),
                                            postDialog.accountModel.item(row, AccountListModel.EmailRole),
                                            postDialog.accountModel.item(row, AccountListModel.AccessJwtRole),
                                            postDialog.accountModel.item(row, AccountListModel.RefreshJwtRole))
                    createRecord.clear()
                    createRecord.setText(postText.text)
                    createRecord.setPostLanguages(postDialog.accountModel.item(row, AccountListModel.PostLanguagesRole))
                    if(postType === "reply"){
                        createRecord.setReply(replyCid, replyUri, replyRootCid, replyRootUri)
                    }else if(postType === "quote"){
                        createRecord.setQuote(replyCid, replyUri)
                    }
                    if(embedImagePreview.embedImages.length > 0){
                        createRecord.setImages(embedImagePreview.embedImages.split("\n"))
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

    P.FileDialog {
        id: fileDialog
        title: qsTr("Select contents")
        visible: false
        fileMode : P.FileDialog.OpenFiles
        nameFilters: ["Image files (*.jpg *.jpeg *.png)"
            , "All files (*)"]
        onAccepted: {
            //選択されたファイルをすべて追加
            prevFolder = folder

            var images = embedImagePreview.embedImages.split("\n")
            if(images.length >= 4){
                return
            }
            var new_images = embedImagePreview.embedImages
            for(var i=0; i<files.length; i++){
                if(i >= 4){
                    break
                }
                if(images.indexOf(files[i]) >= 0){
                    continue;
                }
                if(new_images.length > 0){
                    new_images += "\n"
                }
                new_images += files[i]
            }
            embedImagePreview.embedImages = new_images
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
}
