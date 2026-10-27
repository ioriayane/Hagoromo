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
import "../compat"

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
    property var links: []      // プロフィール画面のリンク（AppViewの情報、ラベル付きも含む）

    // 編集中のリンク（Updateでまとめて反映する）
    property var visibleLinks: []   // プロフィールに表示するリンク（並び順どおり）
    property var hiddenLinks: []    // レコードはあるがプロフィールに表示しないリンク
    property var deletedLinkUris: []
    property int editingLinkIndex: -1
    property bool editingLinkVisible: true
    property bool linkOrderChanged: false
    property bool linksLoaded: false
    property bool linksLoadFailed: false
    property bool hiddenLinksExpanded: false
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
    function decorateLinks(items) {
        for(var i=0; i<items.length; i++){
            items[i].title = displayTitle(items[i].rawTitle, items[i].url)
            // 背景色のハイライトはプロフィール画面と同じく支援サイトのリンク、未保存の変更は枠線で示す
            items[i].supportProvider = systemTool.isSupportProviderLink(items[i].url)
            items[i].supportProviderLogo = systemTool.supportProviderLogo(items[i].url)
            items[i].highlight = items[i].supportProvider
            items[i].edited = (items[i].status.length > 0)
        }
        return items
    }
    function setLinks(visible_items, hidden_items) {
        // 表示更新のため配列ごと差し替える
        visibleLinks = decorateLinks(visible_items)
        hiddenLinks = decorateLinks(hidden_items)
    }
    function makeEditItem(record) {
        // AppViewの情報があればCDNのアイコンとラベルを使う
        var icon = record.iconUrl
        var labels = record.labels.slice()
        for(var i=0; i<links.length; i++){
            if(links[i].uri !== record.uri){
                continue
            }
            if(links[i].icon && links[i].icon.length > 0){
                icon = links[i].icon
            }
            if(links[i].labels){
                for(var j=0; j<links[i].labels.length; j++){
                    if(labels.indexOf(links[i].labels[j]) < 0){
                        labels.push(links[i].labels[j])
                    }
                }
            }
            break
        }
        return {
            "uri": record.uri,
            "cid": record.cid,
            "url": record.url,
            "rawTitle": record.rawTitle,
            "icon": icon,
            "iconLocal": "",
            "iconChanged": false,
            "status": "",
            "labels": labels
        }
    }
    function moveLink(from, to) {
        if(from < 0 || from >= visibleLinks.length || to < 0 || to >= visibleLinks.length){
            return
        }
        var items = visibleLinks.slice()
        var item = items.splice(from, 1)[0]
        items.splice(to, 0, item)
        linkOrderChanged = true
        setLinks(items, hiddenLinks.slice())
    }
    function setLinkVisible(index, visible) {
        var from = visible ? hiddenLinks.slice() : visibleLinks.slice()
        var to = visible ? visibleLinks.slice() : hiddenLinks.slice()
        if(index < 0 || index >= from.length){
            return
        }
        if(visible && to.length >= linksMax){
            return
        }
        var item = from.splice(index, 1)[0]
        if(visible){
            to.push(item)
        }else{
            to.unshift(item)
        }
        linkOrderChanged = true
        if(visible){
            setLinks(to, from)
        }else{
            setLinks(from, to)
        }
    }
    function openLinkEditor(index, visible) {
        var item = visible ? visibleLinks[index] : hiddenLinks[index]
        editingLinkIndex = index
        editingLinkVisible = visible
        profileLinkDialog.editMode = true
        profileLinkDialog.url = item.url
        profileLinkDialog.linkTitle = item.rawTitle
        profileLinkDialog.icon = item.icon
        profileLinkDialog.open()
    }
    function toOperatorItem(item, visible) {
        return {
            "uri": item.uri,
            "cid": item.cid,
            "url": item.url,
            "title": item.rawTitle,
            "iconLocal": item.iconLocal,
            "iconChanged": item.iconChanged,
            "status": item.status,
            "visible": visible
        }
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
        deletedLinkUris = []
        linkOrderChanged = false
        linksLoaded = false
        linksLoadFailed = false
        hiddenLinksExpanded = false
        setLinks([], [])
        linkLoader.setAccount(account.uuid)
        linkLoader.requestProfileLinks()
    }
    onClosed: {
        setLinks([], [])
        deletedLinkUris = []
        linkOrderChanged = false
        linksLoaded = false
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
        // リンクのレコード一覧の取得用（プロフィール更新とは状態を分ける）
        id: linkLoader
        onFinishedRequestProfileLinks: (success, visible_links, hidden_links) => {
            if(!editProfileDialog.visible){
                return
            }
            if(!success){
                editProfileDialog.linksLoadFailed = true
                return
            }
            var visible_items = []
            var hidden_items = []
            for(var i=0; i<visible_links.length; i++){
                visible_items.push(editProfileDialog.makeEditItem(visible_links[i]))
            }
            for(i=0; i<hidden_links.length; i++){
                hidden_items.push(editProfileDialog.makeEditItem(hidden_links[i]))
            }
            editProfileDialog.setLinks(visible_items, hidden_items)
            editProfileDialog.linksLoaded = true
        }
        onErrorOccurred: (code, message) => editProfileDialog.errorOccurred(account.uuid, code, message)
    }
    RecordOperator {
        id: recordOperator
        onFinished: (success) => {
                        if(success){
                            var links = editProfileDialog.links
                            if(editProfileDialog.linksLoaded){
                                // 新規作成・更新したリンクのuri/cidを反映してプロフィール画面へ返す
                                // （setProfileLinksには表示するリンクを先頭に渡している）
                                var results = recordOperator.profileLinks()
                                links = []
                                for(var i=0; i<editProfileDialog.visibleLinks.length; i++){
                                    var item = editProfileDialog.visibleLinks[i]
                                    var result = (i < results.length) ? results[i] : {"uri": item.uri, "cid": item.cid}
                                    links.push({
                                                   "uri": result.uri,
                                                   "cid": result.cid,
                                                   "url": item.url,
                                                   "title": item.title,
                                                   "rawTitle": item.rawTitle,
                                                   "icon": item.icon,
                                                   "labels": item.labels,
                                                   "supportProvider": item.supportProvider,
                                                   "supportProviderLogo": item.supportProviderLogo,
                                                   "highlight": item.supportProvider
                                               })
                                }
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

    component LinkRow: RowLayout {
        id: linkRow
        property var item
        property int linkIndex: -1
        property bool linkVisible: true
        spacing: 5
        LinkLabel {
            Layout.maximumWidth: 230 * AdjustedValues.ratio
            url: linkRow.item.url
            text: linkRow.item.title
            iconSource: linkRow.item.icon
            providerLogo: linkRow.item.supportProviderLogo
            highlight: linkRow.item.highlight
            edited: linkRow.item.edited
            fontPointSize: AdjustedValues.f8
            clip: true
            onClicked: (url) => editProfileDialog.openLinkEditor(linkRow.linkIndex, linkRow.linkVisible)
        }
        Image {
            Layout.preferredWidth: AdjustedValues.i16
            Layout.preferredHeight: AdjustedValues.i16
            visible: linkRow.item.labels.length > 0
            source: "../images/warning.png"
            layer.enabled: true
            layer.effect: ColorOverlayC {
                color: Material.color(Material.Orange)
            }
            HoverHandler {
                id: labelsHover
            }
            ToolTip.visible: labelsHover.hovered
            ToolTip.text: qsTr("Labels") + ": " + linkRow.item.labels.join(", ")
        }
        Label {
            Layout.fillWidth: true
            font.pointSize: AdjustedValues.f8
            color: Material.color(Material.Grey)
            elide: Text.ElideRight
            text: linkRow.item.url
        }
        IconButton {
            Layout.preferredWidth: AdjustedValues.b24
            Layout.preferredHeight: AdjustedValues.b24
            visible: linkRow.linkVisible
            enabled: linkRow.linkIndex > 0
            iconSource: "../images/expand_less.png"
            iconSize: AdjustedValues.i16
            onClicked: editProfileDialog.moveLink(linkRow.linkIndex, linkRow.linkIndex - 1)
        }
        IconButton {
            Layout.preferredWidth: AdjustedValues.b24
            Layout.preferredHeight: AdjustedValues.b24
            visible: linkRow.linkVisible
            enabled: linkRow.linkIndex < editProfileDialog.visibleLinks.length - 1
            iconSource: "../images/expand_more.png"
            iconSize: AdjustedValues.i16
            onClicked: editProfileDialog.moveLink(linkRow.linkIndex, linkRow.linkIndex + 1)
        }
        IconButton {
            Layout.preferredWidth: AdjustedValues.b24
            Layout.preferredHeight: AdjustedValues.b24
            enabled: linkRow.linkVisible ||
                     editProfileDialog.visibleLinks.length < editProfileDialog.linksMax
            iconSource: linkRow.linkVisible ? "../images/visibility_off.png" : "../images/visibility_on.png"
            iconSize: AdjustedValues.i16
            onClicked: editProfileDialog.setLinkVisible(linkRow.linkIndex, !linkRow.linkVisible)
            HoverHandler {
                id: visibleHover
            }
            ToolTip.visible: visibleHover.hovered
            ToolTip.text: linkRow.linkVisible ? qsTr("Hide from profile") : qsTr("Show on profile")
        }
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
            BusyIndicator {
                Layout.preferredWidth: AdjustedValues.i16
                Layout.preferredHeight: AdjustedValues.i16
                visible: linkLoader.running
            }
            Label {
                font.pointSize: AdjustedValues.f8
                color: editProfileDialog.visibleLinks.length >= editProfileDialog.linksMax ?
                           Material.color(Material.Orange) : Material.foreground
                text: editProfileDialog.visibleLinks.length + "/" + editProfileDialog.linksMax
            }
            IconButton {
                enabled: !recordOperator.running && editProfileDialog.linksLoaded
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
        Label {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            visible: editProfileDialog.linksLoadFailed
            wrapMode: Text.Wrap
            font.pointSize: AdjustedValues.f8
            color: Material.color(Material.Red)
            text: qsTr("Failed to load links. Links will not be changed.")
        }
        ColumnLayout {
            Layout.preferredWidth: 500 * AdjustedValues.ratio
            Layout.topMargin: 3
            spacing: 3
            enabled: !recordOperator.running && editProfileDialog.linksLoaded
            visible: editProfileDialog.visibleLinks.length > 0
            Repeater {
                model: editProfileDialog.visibleLinks
                LinkRow {
                    Layout.fillWidth: true
                    item: modelData
                    linkIndex: model.index
                    linkVisible: true
                }
            }
        }
        RowLayout {
            Layout.topMargin: 3
            visible: editProfileDialog.hiddenLinks.length > 0
            IconButton {
                Layout.fillWidth: true
                flat: true
                iconSource: editProfileDialog.hiddenLinksExpanded ? "../images/expand_less.png" : "../images/expand_more.png"
                iconSize: AdjustedValues.i16
                iconText: qsTr("Links not shown on profile") + " (" + editProfileDialog.hiddenLinks.length + ")"
                onClicked: editProfileDialog.hiddenLinksExpanded = !editProfileDialog.hiddenLinksExpanded
            }
        }
        ColumnLayout {
            Layout.preferredWidth: 500 * AdjustedValues.ratio
            spacing: 3
            enabled: !recordOperator.running && editProfileDialog.linksLoaded
            visible: editProfileDialog.hiddenLinksExpanded && editProfileDialog.hiddenLinks.length > 0
            Repeater {
                model: editProfileDialog.hiddenLinks
                LinkRow {
                    Layout.fillWidth: true
                    item: modelData
                    linkIndex: model.index
                    linkVisible: false
                }
            }
        }
        Label {
            Layout.fillWidth: true
            Layout.topMargin: 3
            visible: editProfileDialog.linkOrderChanged
            font.pointSize: AdjustedValues.f8
            color: Material.color(Material.Teal)
            text: qsTr("Changes to the links are not saved yet.")
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
                    if(editProfileDialog.linksLoaded){
                        // 表示するリンクを先頭に並べる（結果の対応付けに使う）
                        var items = []
                        for(var i=0; i<editProfileDialog.visibleLinks.length; i++){
                            items.push(editProfileDialog.toOperatorItem(editProfileDialog.visibleLinks[i], true))
                        }
                        for(i=0; i<editProfileDialog.hiddenLinks.length; i++){
                            items.push(editProfileDialog.toOperatorItem(editProfileDialog.hiddenLinks[i], false))
                        }
                        recordOperator.setProfileLinks(items, editProfileDialog.deletedLinkUris)
                    }
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
            var visible_items = editProfileDialog.visibleLinks.slice()
            var hidden_items = editProfileDialog.hiddenLinks.slice()
            var index = editProfileDialog.editingLinkIndex
            if(index < 0){
                var new_item = {
                    "uri": "",
                    "cid": "",
                    "url": url,
                    "rawTitle": linkTitle,
                    "icon": iconLocal,
                    "iconLocal": iconLocal,
                    "iconChanged": true,
                    "status": "new",
                    "labels": []
                }
                if(visible_items.length < editProfileDialog.linksMax){
                    visible_items.push(new_item)
                }else{
                    // 上限に達しているときは未表示として追加する
                    hidden_items.unshift(new_item)
                    editProfileDialog.hiddenLinksExpanded = true
                }
            }else{
                var items = editProfileDialog.editingLinkVisible ? visible_items : hidden_items
                if(index < items.length){
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
            }
            editProfileDialog.setLinks(visible_items, hidden_items)
        }
        onRequestDelete: {
            var visible_items = editProfileDialog.visibleLinks.slice()
            var hidden_items = editProfileDialog.hiddenLinks.slice()
            var items = editProfileDialog.editingLinkVisible ? visible_items : hidden_items
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
            editProfileDialog.linkOrderChanged = true
            editProfileDialog.setLinks(visible_items, hidden_items)
        }
    }
}
