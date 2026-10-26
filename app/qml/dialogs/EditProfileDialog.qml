import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import Qt.labs.platform 1.1 as P

import tech.relog.hagoromo.recordoperator 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"

Dialog {
    id: editProfileDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Edit my profile")
    closePolicy: Popup.NoAutoClose

    property string avatar: ""
    property string banner: ""
    property string displayName: ""
    property string description: ""
    property string pronouns: ""
    property string website: ""
    property var links: []

    // 編集中のリンク（Updateでまとめて反映する）
    property var editLinks: []
    property var deletedLinkUris: []
    property int editingLinkIndex: -1
    property bool linkOrderChanged: false
    readonly property int linksMax: 10

    property alias account: account
    Account {
        id: account
    }
    signal errorOccurred(string account_uuid, string code, string message)
    signal updatedProfile(string did, string avatar, string banner, string display_name, string description, string pronouns, string website, var links)

    function displayTitle(title, url) {
        if(title.length > 0){
            return title
        }
        return url.replace(/^https?:\/\/([^\/?#:]+).*$/, "$1")
    }
    function setEditLinks(items) {
        // 表示更新のため配列ごと差し替える
        for(var i=0; i<items.length; i++){
            items[i].title = displayTitle(items[i].rawTitle, items[i].url)
            items[i].highlight = (items[i].status.length > 0)
        }
        editLinks = items
    }
    function moveLink(from, to) {
        if(from < 0 || from >= editLinks.length || to < 0 || to >= editLinks.length){
            return
        }
        var items = editLinks.slice()
        var item = items.splice(from, 1)[0]
        items.splice(to, 0, item)
        linkOrderChanged = true
        setEditLinks(items)
    }
    function openLinkEditor(index) {
        var item = editLinks[index]
        editingLinkIndex = index
        profileLinkDialog.editMode = true
        profileLinkDialog.url = item.url
        profileLinkDialog.linkTitle = item.rawTitle
        profileLinkDialog.icon = item.icon
        profileLinkDialog.open()
    }

    onOpened: {
        if(account.uuid.length === 0){
            return
        }
        recordOperator.setAccount(account.uuid)
        avatarImage.source = editProfileDialog.avatar
        bannerImage.source = editProfileDialog.banner
        displayNameText.text = editProfileDialog.displayName
        descriptionText.text = editProfileDialog.description
        pronounsText.text = editProfileDialog.pronouns
        var items = []
        for(var i=0; i<editProfileDialog.links.length; i++){
            var link = editProfileDialog.links[i]
            items.push({
                           "uri": link.uri,
                           "cid": link.cid,
                           "url": link.url,
                           "rawTitle": link.rawTitle !== undefined ? link.rawTitle : link.title,
                           "icon": link.icon,
                           "iconLocal": "",
                           "iconChanged": false,
                           "status": ""
                       })
        }
        deletedLinkUris = []
        linkOrderChanged = false
        setEditLinks(items)
    }
    onClosed: {
        editLinks = []
        deletedLinkUris = []
        linkOrderChanged = false
        pronounsText.clear()
        displayNameText.clear()
        descriptionText.clear()
        avatarImage.source = ""
        bannerImage.source = ""
    }

    Shortcut {  // Close
        // DialogのclosePolicyでEscで閉じられるけど、そのうち編集中の確認ダイアログを
        // 入れたいので別でイベント処理をする。onClosedで閉じるをキャンセルできなさそうなので。
        enabled: editProfileDialog.visible && !addButton.enabled
        sequence: "Esc"
        onActivated: editProfileDialog.close()
    }

    SystemTool {
        id: systemTool
    }
    RecordOperator {
        id: recordOperator
        onFinished: (success) => {
                        if(success){
                            // 新規作成・更新したリンクのuri/cidを反映してプロフィール画面へ返す
                            var results = recordOperator.profileLinks()
                            var links = []
                            for(var i=0; i<editProfileDialog.editLinks.length; i++){
                                var item = editProfileDialog.editLinks[i]
                                var result = (i < results.length) ? results[i] : {"uri": item.uri, "cid": item.cid}
                                links.push({
                                               "uri": result.uri,
                                               "cid": result.cid,
                                               "url": item.url,
                                               "title": item.title,
                                               "rawTitle": item.rawTitle,
                                               "icon": item.icon,
                                               "highlight": false
                                           })
                            }
                            editProfileDialog.accept()
                            editProfileDialog.updatedProfile(
                                account.did, avatarImage.source, bannerImage.source, displayNameText.text,
                                descriptionText.text, pronounsText.text, editProfileDialog.website, links
                                )
                        }
                    }
        onErrorOccurred: (code, message) => editProfileDialog.errorOccurred(account.uuid, code, message)
    }

    ColumnLayout {
        spacing: 0
        Label {
            Layout.fillWidth: true
            font.pointSize: AdjustedValues.f10
            text: account.handle
            elide: Text.ElideRight
        }

        Rectangle {
            Layout.preferredWidth: 500 * AdjustedValues.ratio
            Layout.preferredHeight: 80
            color: Material.frameColor
            ImageWithIndicator {
                id: bannerImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                Rectangle {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: 2
                    width: AdjustedValues.i24
                    height: AdjustedValues.i24
                    radius: width / 2
                    border.color: "black"
                    border.width: 1
                    Image {
                        anchors.fill: parent
                        anchors.margins: 5
                        source: "../images/edit.png"
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: (mouse) => {
                                   imageClipDialog.target = "banner"
                                   imageClipDialog.squareMode = false
                                   fileDialog.open()
                               }
                }
            }
        }
        RowLayout {
            spacing: 10 * AdjustedValues.ratio
            Rectangle {
                Layout.preferredWidth: AdjustedValues.i48
                Layout.preferredHeight: AdjustedValues.i48
                color: Material.frameColor
                radius: width / 2
                AvatarImage {
                    id: avatarImage
                    anchors.fill: parent
                    onClicked: (mouse) => {
                                   imageClipDialog.target = "avatar"
                                   imageClipDialog.squareMode = true
                                   fileDialog.open()
                               }
                    Rectangle {
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.margins: 2
                        width: AdjustedValues.i18
                        height: AdjustedValues.i18
                        radius: width / 2
                        border.color: "black"
                        border.width: 1
                        Image {
                            anchors.fill: parent
                            anchors.margins: 3
                            source: "../images/edit.png"
                        }
                    }
                }
            }
            ColumnLayout {
                spacing: 0
                Label {
                    Layout.fillWidth: true
                    Layout.topMargin: 5
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("Display Name")
                }
                TextField  {
                    id: displayNameText
                    Layout.fillWidth: true
                    enabled: !recordOperator.running
                    selectByMouse: true
                    font.pointSize: AdjustedValues.f10
                    property int realTextLength: systemTool.countText(text)
                }
            }
        }
        Label {
            Layout.fillWidth: true
            Layout.topMargin: 5
            font.pointSize: AdjustedValues.f10
            text: qsTr("Description")
        }
        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: 120 * AdjustedValues.ratio
            Layout.topMargin: 0
            TextArea {
                id: descriptionText
                verticalAlignment: TextInput.AlignTop
                topInset: 0
                enabled: !recordOperator.running
                wrapMode: TextInput.WordWrap
                selectByMouse: true
                font.pointSize: AdjustedValues.f10
                property int realTextLength: systemTool.countText(text)
            }
        }
        RowLayout {
            Label {
                Layout.fillWidth: true
                Layout.topMargin: 5
                font.pointSize: AdjustedValues.f10
                text: qsTr("Pronouns")
            }
            Label {
                Layout.alignment: Qt.AlignTop
                font.pointSize: AdjustedValues.f8
                text: 256 - descriptionText.realTextLength
            }
        }
        TextField  {
            id: pronounsText
            Layout.fillWidth: true
            enabled: !recordOperator.running
            selectByMouse: true
            font.pointSize: AdjustedValues.f10
            property int realTextLength: systemTool.countText(text)
        }

        RowLayout {
            Layout.topMargin: 5
            Label {
                Layout.fillWidth: true
                font.pointSize: AdjustedValues.f10
                text: qsTr("Links")
            }
            Label {
                font.pointSize: AdjustedValues.f8
                color: editProfileDialog.editLinks.length >= editProfileDialog.linksMax ?
                           Material.color(Material.Orange) : Material.foreground
                text: editProfileDialog.editLinks.length + "/" + editProfileDialog.linksMax
            }
            IconButton {
                enabled: !recordOperator.running &&
                         editProfileDialog.editLinks.length < editProfileDialog.linksMax
                iconSource: "../images/add.png"
                iconSize: AdjustedValues.i16
                iconText: qsTr("Add link")
                onClicked: {
                    editProfileDialog.editingLinkIndex = -1
                    profileLinkDialog.editMode = false
                    profileLinkDialog.url = ""
                    profileLinkDialog.linkTitle = ""
                    profileLinkDialog.icon = ""
                    profileLinkDialog.open()
                }
            }
        }
        ColumnLayout {
            Layout.preferredWidth: 500 * AdjustedValues.ratio
            Layout.topMargin: 3
            spacing: 3
            enabled: !recordOperator.running
            visible: editProfileDialog.editLinks.length > 0
            Repeater {
                model: editProfileDialog.editLinks
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    LinkLabel {
                        Layout.maximumWidth: 250 * AdjustedValues.ratio
                        url: modelData.url
                        text: modelData.title
                        iconSource: modelData.icon
                        highlight: modelData.highlight
                        fontPointSize: AdjustedValues.f8
                        clip: true
                        onClicked: (url) => editProfileDialog.openLinkEditor(model.index)
                    }
                    Label {
                        Layout.fillWidth: true
                        font.pointSize: AdjustedValues.f8
                        color: Material.color(Material.Grey)
                        elide: Text.ElideRight
                        text: modelData.url
                    }
                    IconButton {
                        Layout.preferredWidth: AdjustedValues.b24
                        Layout.preferredHeight: AdjustedValues.b24
                        enabled: model.index > 0
                        iconSource: "../images/expand_less.png"
                        iconSize: AdjustedValues.i16
                        onClicked: editProfileDialog.moveLink(model.index, model.index - 1)
                    }
                    IconButton {
                        Layout.preferredWidth: AdjustedValues.b24
                        Layout.preferredHeight: AdjustedValues.b24
                        enabled: model.index < editProfileDialog.editLinks.length - 1
                        iconSource: "../images/expand_more.png"
                        iconSize: AdjustedValues.i16
                        onClicked: editProfileDialog.moveLink(model.index, model.index + 1)
                    }
                }
            }
        }
        Label {
            Layout.fillWidth: true
            Layout.topMargin: 3
            visible: editProfileDialog.linkOrderChanged
            font.pointSize: AdjustedValues.f8
            color: Material.color(Material.Teal)
            text: qsTr("The order of links is not saved yet.")
        }

        RowLayout {
            Button {
                Layout.alignment: Qt.AlignLeft
                flat: true
                font.pointSize: AdjustedValues.f10
                text: qsTr("Cancel")
                onClicked: editProfileDialog.reject()
            }
            Item {
                Layout.fillWidth: true
                height: 1
            }
            Button {
                id: addButton
                Layout.alignment: Qt.AlignRight
                font.pointSize: AdjustedValues.f10
                enabled: displayNameText.realTextLength <= 64 &&
                         descriptionText.realTextLength <= 256 &&
                         !recordOperator.running
                text: qsTr("Update")
                onClicked: {
                    recordOperator.clear()
                    recordOperator.setProfileLinks(editProfileDialog.editLinks.map(function(item){
                        return {
                            "uri": item.uri,
                            "cid": item.cid,
                            "url": item.url,
                            "title": item.rawTitle,
                            "iconLocal": item.iconLocal,
                            "iconChanged": item.iconChanged,
                            "status": item.status
                        }
                    }), editProfileDialog.deletedLinkUris)
                    recordOperator.updateProfile(avatarImage.source, bannerImage.source,
                                                 descriptionText.text, displayNameText.text,
                                                 pronounsText.text, editProfileDialog.website)
                }
                BusyIndicator {
                    anchors.fill: parent
                    anchors.margins: 3
                    visible: recordOperator.running
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
            prevFolder = folder
            imageClipDialog.embedImage = file
            imageClipDialog.open()
        }
        property string prevFolder
    }
    ImageClipDialog {
        id: imageClipDialog
        property string target: ""
        onRejected: embedImage = ""
        onAccepted: {
            console.log("Selected=" + selectedX + "," + selectedY + "," + selectedWidth + "," + selectedHeight)
            var clipped = systemTool.clipImage(embedImage,
                                               selectedX, selectedY,
                                               selectedWidth, selectedHeight)
            if(target === "avatar"){
                avatarImage.source = clipped
            }else if(target === "banner"){
                bannerImage.source = clipped
            }
        }
    }
    ProfileLinkDialog {
        id: profileLinkDialog
        onAccepted: {
            var items = editProfileDialog.editLinks.slice()
            var index = editProfileDialog.editingLinkIndex
            if(index < 0){
                items.push({
                               "uri": "",
                               "cid": "",
                               "url": url,
                               "rawTitle": linkTitle,
                               "icon": iconLocal,
                               "iconLocal": iconLocal,
                               "iconChanged": true,
                               "status": "new"
                           })
            }else if(index < items.length){
                var item = items[index]
                if(item.url !== url || item.rawTitle !== linkTitle || iconChanged){
                    item.url = url
                    item.rawTitle = linkTitle
                    if(iconChanged){
                        item.icon = iconLocal
                        item.iconLocal = iconLocal
                        item.iconChanged = true
                    }
                    if(item.status !== "new"){
                        item.status = "modified"
                    }
                }
            }
            editProfileDialog.setEditLinks(items)
        }
        onRequestDelete: {
            var items = editProfileDialog.editLinks.slice()
            var index = editProfileDialog.editingLinkIndex
            if(index < 0 || index >= items.length){
                return
            }
            if(items[index].status !== "new" && items[index].uri.length > 0){
                var uris = editProfileDialog.deletedLinkUris.slice()
                uris.push(items[index].uri)
                editProfileDialog.deletedLinkUris = uris
            }
            items.splice(index, 1)
            editProfileDialog.setEditLinks(items)
        }
    }
}
