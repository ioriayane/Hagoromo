import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import Qt.labs.settings 1.1

import tech.relog.hagoromo.accountlistmodel 1.0
import tech.relog.hagoromo.columnlistmodel 1.0
import tech.relog.hagoromo.systemtool 1.0

import "controls"
import "dialogs"
import "view"

ApplicationWindow {
    id: appWindow
    width: 800
    height: 480
    visible: true
    title: "羽衣 -Hagoromo-"

    Material.theme: settingDialog.settings.theme
    Material.accent: settingDialog.settings.accent

    Settings {
        //        property alias x: appWindow.x
        //        property alias y: appWindow.y
        property alias width: appWindow.width
        property alias height: appWindow.height
    }

    SystemTool {
        id: systemTool
    }

    Shortcut {  // Post
        enabled: !postDialog.visible
        context: Qt.ApplicationShortcut
        sequence: "n"
        onActivated: postDialog.open()
    }
    Shortcut {  // Search
        enabled: !searchDialog.visible
        context: Qt.ApplicationShortcut
        sequence: "s"
        onActivated: searchDialog.open()
    }

    SettingDialog {
        id: settingDialog
    }

    PostDialog {
        id: postDialog
        accountModel: accountListModel
    }

    SearchDialog {
        id: searchDialog
        accountModel: accountListModel
        onAccepted: {
            console.log("selectedAccountIndex=" + selectedAccountIndex + ", searchType=" + searchType)
            var component_type = 2
            var column_name = qsTr("Search posts")
            if(searchType === "users"){
                component_type = 3
                column_name = qsTr("Search users")
            }
            columnManageModel.append(accountListModel.item(selectedAccountIndex, AccountListModel.UuidRole),
                                     component_type, false, 300000, 350, column_name, searchDialog.searchText)
        }
    }

    AddColumnDialog {
        id: addColumnDialog
        accountModel: accountListModel
        onAccepted: {
            console.log("Add column\n  selectedAccountIndex=" + selectedAccountIndex +
                        "\n  selectedType=" + selectedType +
                        "\n  selectedName=" + selectedName +
                        "\n  selectedUri=" + selectedUri)
            columnManageModel.append(accountListModel.item(selectedAccountIndex, AccountListModel.UuidRole),
                                     selectedType, false, 300000, 400, selectedName, selectedUri)
        }
        onOpenDiscoverFeeds: (account_index) => {
                                 discoverFeedsDialog.account.uuid = accountListModel.item(account_index, AccountListModel.UuidRole)
                                 discoverFeedsDialog.account.service = accountListModel.item(account_index, AccountListModel.ServiceRole)
                                 discoverFeedsDialog.account.did = accountListModel.item(account_index, AccountListModel.DidRole)
                                 discoverFeedsDialog.account.handle = accountListModel.item(account_index, AccountListModel.HandleRole)
                                 discoverFeedsDialog.account.email = accountListModel.item(account_index, AccountListModel.EmailRole)
                                 discoverFeedsDialog.account.accessJwt = accountListModel.item(account_index, AccountListModel.AccessJwtRole)
                                 discoverFeedsDialog.account.refreshJwt = accountListModel.item(account_index, AccountListModel.RefreshJwtRole)
                                 discoverFeedsDialog.account.avatar = accountListModel.item(account_index, AccountListModel.AvatarRole)
                                 discoverFeedsDialog.open()

                                 addColumnDialog.reject()
                             }
    }

    AccountDialog {
        id: accountDialog
        accountModel: accountListModel
        onClosed: accountListModel.syncColumn()
    }

    DiscoverFeedsDialog {
        id: discoverFeedsDialog
        onAccepted: {
            columnManageModel.append(discoverFeedsDialog.account.uuid,
                                     4, false, 300000, 400, selectedName, selectedUri)
        }
    }

    ColumnSettingDialog {
        id: columnsettingDialog

        function openWithKey(key) {
            columnKey = key
            var i = columnManageModel.indexOf(columnKey)
            console.log("open column setting dialog:" + i + ", " + columnKey)
            if(i >= 0){
                autoLoadingCheckbox.checked = columnManageModel.item(i, ColumnListModel.AutoLoadingRole)
                autoLoadingIntervalCombo.setByValue(columnManageModel.item(i, ColumnListModel.LoadingIntervalRole))
                columnWidthSlider.value = columnManageModel.item(i, ColumnListModel.WidthRole)
                componentType = columnManageModel.item(i, ColumnListModel.ComponentTypeRole)

                visibleLikeCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleLikeRole)
                visibleRepostCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleRepostRole)
                visibleFollowCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleFollowRole)
                visibleMentionCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleMentionRole)
                visibleReplyCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleReplyRole)
                visibleQuoteCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleQuoteRole)

                open()
            }
        }

        onAccepted: {
            var i = columnManageModel.indexOf(columnKey)
            if(i >= 0){
                columnManageModel.update(i, ColumnListModel.AutoLoadingRole, autoLoadingCheckbox.checked)
                columnManageModel.update(i, ColumnListModel.LoadingIntervalRole, autoLoadingIntervalCombo.currentValue)
                columnManageModel.update(i, ColumnListModel.WidthRole, columnWidthSlider.value)

                columnManageModel.update(i, ColumnListModel.VisibleLikeRole, visibleLikeCheckBox.checked)
                columnManageModel.update(i, ColumnListModel.VisibleRepostRole, visibleRepostCheckBox.checked)
                columnManageModel.update(i, ColumnListModel.VisibleFollowRole, visibleFollowCheckBox.checked)
                columnManageModel.update(i, ColumnListModel.VisibleMentionRole, visibleMentionCheckBox.checked)
                columnManageModel.update(i, ColumnListModel.VisibleReplyRole, visibleReplyCheckBox.checked)
                columnManageModel.update(i, ColumnListModel.VisibleQuoteRole, visibleQuoteCheckBox.checked)

                repeater.updateSetting()
            }
        }
    }

    ReportPostDialog {
        id: reportDialog
    }
    ReportAccountDialog {
        id: reportAccountDialog
    }

    MessageDialog {
        id: messageDialog
    }

    // アカウントの管理
    // カラムとこのモデルとの紐付けはインデックスで実施する
    // アカウント管理で内容が変更されたときにカラムとインデックスの関係が崩れるのでuuidで確認する
    AccountListModel {
        id: accountListModel
        onAppendedAccount: (row) => {
                               console.log("onAppendedAccount:" + row)
                           }
        onUpdatedAccount: (row, uuid) => {
                              console.log("onUpdatedAccount:" + row + ", " + uuid)
                              // カラムを更新しにいく
                              repeater.updateAccount(uuid)
                          }

        onAllFinished: {
            // すべてのアカウント情報の認証が終わったのでカラムを復元（成功しているとは限らない）
            console.log("allFinished()" + accountListModel.count)
            if(columnManageModel.rowCount() === 0){
                columnManageModel.load()
            }
        }
        onErrorOccured: (message) => {console.log(message)}

        function syncColumn(){
            // アカウント一覧にないものを消す
            var exist = false
            for(var i=columnManageModel.rowCount()-1; i>=0; i--){
                exist = false
                for(var a=0; a<accountListModel.rowCount();a++){
                    if(columnManageModel.item(i, ColumnListModel.AccountUuidRole) === accountListModel.item(a, AccountListModel.UuidRole)){
                        exist = true
                    }
                }
                if(exist === false){
                    columnManageModel.remove(i)
                }
            }
        }
    }

    Component {
        id: columnView
        ColumnView {
            fontSizeRatio: settingDialog.settings.fontSizeRatio

            onRequestReply: (account_uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) => {
                                console.log(account_uuid + ",\n" +
                                            cid + ", "+ uri + ",\n" +
                                            reply_root_cid + ", "+ reply_root_uri + ",\n" +
                                            avatar + ",\n" +
                                            display_name + ", "+ handle + ", "+ indexed_at + ",\n"+ text)
                                postDialog.postType = "reply"
                                postDialog.defaultAccountUuid = account_uuid
                                postDialog.replyCid = cid
                                postDialog.replyUri = uri
                                postDialog.replyRootCid = reply_root_cid
                                postDialog.replyRootUri = reply_root_uri
                                postDialog.replyAvatar = avatar
                                postDialog.replyDisplayName = display_name
                                postDialog.replyHandle = handle
                                postDialog.replyIndexedAt = indexed_at
                                postDialog.replyText = text
                                postDialog.open()
                            }
            onRequestQuote: (account_uuid, cid, uri, avatar, display_name, handle, indexed_at, text) => {
                                postDialog.postType = "quote"
                                postDialog.defaultAccountUuid = account_uuid
                                postDialog.replyCid = cid
                                postDialog.replyUri = uri
                                postDialog.replyRootCid = ""
                                postDialog.replyRootUri = ""
                                postDialog.replyAvatar = avatar
                                postDialog.replyDisplayName = display_name
                                postDialog.replyHandle = handle
                                postDialog.replyIndexedAt = indexed_at
                                postDialog.replyText = text
                                postDialog.open()
                            }
            onRequestMention: (account_uuid, handle) => {
                                  postDialog.postType = "normal"
                                  postDialog.defaultAccountUuid = account_uuid
                                  postDialog.postText.text = handle + " "
                                  postDialog.open()
                              }
            onRequestViewAuthorFeed: (account_uuid, did, handle) => {
                                         columnManageModel.append(account_uuid, 5, false, 300000, 350, handle, did)
                                     }
            onRequestViewImages: (index, paths) => imageFullView.open(index, paths)
            onRequestViewGeneratorFeed: (account_uuid, name, uri) =>
                                        columnManageModel.append(account.uuid, 4, false, 300000, 400, name, uri)

            onRequestReportPost: (account_uuid, uri, cid) => {
                                     var row = accountListModel.indexAt(account_uuid)
                                     if(row >= 0){
                                         reportDialog.targetUri = uri
                                         reportDialog.targetCid = cid
                                         reportDialog.account.service = accountListModel.item(row, AccountListModel.ServiceRole)
                                         reportDialog.account.did = accountListModel.item(row, AccountListModel.DidRole)
                                         reportDialog.account.handle = accountListModel.item(row, AccountListModel.HandleRole)
                                         reportDialog.account.email = accountListModel.item(row, AccountListModel.EmailRole)
                                         reportDialog.account.accessJwt = accountListModel.item(row, AccountListModel.AccessJwtRole)
                                         reportDialog.account.refreshJwt = accountListModel.item(row, AccountListModel.RefreshJwtRole)
                                         reportDialog.account.avatar = accountListModel.item(row, AccountListModel.AvatarRole)
                                         reportDialog.open()
                                     }
                                 }
            onRequestReportAccount: (account_uuid, did) => {
                                        var row = accountListModel.indexAt(account_uuid)
                                        if(row >= 0){
                                            reportAccountDialog.targetDid = did
                                            reportAccountDialog.account.service = accountListModel.item(row, AccountListModel.ServiceRole)
                                            reportAccountDialog.account.did = accountListModel.item(row, AccountListModel.DidRole)
                                            reportAccountDialog.account.handle = accountListModel.item(row, AccountListModel.HandleRole)
                                            reportAccountDialog.account.email = accountListModel.item(row, AccountListModel.EmailRole)
                                            reportAccountDialog.account.accessJwt = accountListModel.item(row, AccountListModel.AccessJwtRole)
                                            reportAccountDialog.account.refreshJwt = accountListModel.item(row, AccountListModel.RefreshJwtRole)
                                            reportAccountDialog.account.avatar = accountListModel.item(row, AccountListModel.AvatarRole)
                                            reportAccountDialog.open()
                                        }
                                    }

            onRequestMoveToLeft: (key) => {
                                     console.log("move to left:" + key)
                                     columnManageModel.move(key, ColumnListModel.MoveLeft)
                                     repeater.updatePosition()
                                 }
            onRequestMoveToRight: (key) => {
                                      console.log("move to right:" + key)
                                      columnManageModel.move(key, ColumnListModel.MoveRight)
                                      repeater.updatePosition()
                                  }
            onRequestRemove: (key) => {
                                 console.log("remove column:" + key)
                                 columnManageModel.removeByKey(key)
                                 repeater.updatePosition()
                             }
            onRequestDisplayOfColumnSetting: (key) => columnsettingDialog.openWithKey(key)
            onHoveredLinkChanged: hoveredLinkFrame.text = hoveredLink
        }
    }

    ColumnLayout {
        anchors.right: rootLayout.right
        anchors.bottom: rootLayout.bottom
        anchors.rightMargin: 5
        anchors.bottomMargin: scrollView.ScrollBar.horizontal.height + 5
        Label {
            Layout.alignment: Qt.AlignRight
            font.pointSize: 10
            color: Material.color(Material.Grey)
            text: "羽衣 -Hagoromo-"
        }
        Label {
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: 5
            font.pointSize: 8
            color: Material.color(Material.Grey)
            text: "Version : " + systemTool.applicationVersion
        }
        Label {
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: 5
            font.pointSize: 8
            color: Material.color(Material.Grey)
            text: "build on Qt " + systemTool.qtVersion
        }
        Label {
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: 5
            font.pointSize: 8
            color: Material.color(Material.Grey)
            text: "© 2023 Iori Ayane"
        }
    }

    RowLayout {
        id: rootLayout
        anchors.fill: parent
        spacing: 0
        Item {
            Layout.fillHeight: true
            Layout.minimumWidth: 48
            Layout.preferredWidth: 48
            Layout.maximumWidth: 96
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 1
                spacing: 0

                IconButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    enabled: accountListModel.count > 0
                    display: AbstractButton.IconOnly
                    iconSource: "images/edit.png"
                    //                    iconText: qsTr("New Post")
                    onClicked: postDialog.open()
                }

                IconButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    enabled: accountListModel.count > 0
                    display: AbstractButton.IconOnly
                    iconSource: "images/search.png"
                    onClicked: searchDialog.open()
                }

                Item {
                    Layout.preferredWidth: 1
                    Layout.fillHeight: true
                }

                IconButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    enabled: accountListModel.count > 0
                    display: AbstractButton.IconOnly
                    iconSource: "images/column.png"
                    //                    iconText: qsTr("Add column")

                    onClicked: addColumnDialog.open()
                }

                IconButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    display: AbstractButton.IconOnly
                    iconSource: "images/account.png"
                    //                    iconText: qsTr("Account")

                    onClicked: accountDialog.open()
                }

                IconButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    display: AbstractButton.IconOnly
                    iconSource: "images/settings.png"
                    //                    iconText: qsTr("Settings")

                    onClicked: settingDialog.open()
                }
            }
        }
        Rectangle {
            Layout.preferredWidth: 3
            Layout.fillHeight: true
            color: Material.dividerColor
        }
        ScrollView {
            id: scrollView
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 2
            ScrollBar.vertical.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.interactive: false
            ScrollBar.vertical.snapMode: ScrollBar.SnapAlways
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
            clip: true

            property int childHeight: scrollView.height - scrollView.ScrollBar.horizontal.height
            contentHeight: childHeight

            Repeater {
                id: repeater
                model: ColumnListModel {
                    //カラムの情報管理
                    id: columnManageModel
                }

                function updateSetting() {
                    var w = 0
                    for(var i=0; i<repeater.count; i++){
                        var item = repeater.itemAt(i)   //ここのitemはloader自身
                        item.setSettings()
                        w += item.width + item.anchors.leftMargin
                    }
                    scrollView.contentWidth = w
                }

                function updatePosition() {
                    // モデルの順番を入れ替えるとLoader側が対応できないのと
                    // 最左にくるものから処理しないとレイアウトが循環して矛盾するため
                    // カラムの管理順ではなくポジションの順番で処理する
                    var w = 0
                    var row_list = columnManageModel.getRowListInOrderOfPosition()
                    for(var i=0; i<row_list.length; i++){
                        var item = repeater.itemAt(row_list[i])   //ここのitemはloader自身
                        item.setLayout()
                        w += item.width + item.anchors.leftMargin
                    }
                    scrollView.contentWidth = w
                }

                function updateAccount(account_uuid){
                    for(var i=0; i<repeater.count; i++){
                        var item = repeater.itemAt(i)   //ここのitemはloader自身
                        if(item.item.account.uuid === account_uuid){
                            var row = accountListModel.indexAt(item.item.account.uuid)
                            if(row >= 0){
                                console.log("Update column : col=" + i + ", a_row=" + row)
                                item.setAccount(row)
                                item.item.reflect()
                            }
                        }
                    }
                }

                Loader {
                    id: loader
                    height: scrollView.childHeight
                    width: model.width
                    sourceComponent: columnView

                    onLoaded: {
                        // Loaderで読み込んだComponentにアカウント情報など設定する
                        var row = accountListModel.indexAt(model.accountUuid)
                        console.log("(1) loader:" + row + ", " + model.accountUuid)
                        if(row < 0){
                            return
                        }

                        item.columnKey = model.key
                        item.componentType = model.componentType
                        item.account.uuid = model.accountUuid
                        setSettings()
                        setAccount(row)
                        item.load()

                        if(model.index === columnManageModel.rowCount() - 1){
                            // 最後の時にレイアウトを設定する
                            // 読み込んでいる過程では左がいない場合がある
                            repeater.updatePosition()
                        }
                    }

                    function setLayout() {
                        var left_pos = columnManageModel.getPreviousRow(model.index)
                        if(left_pos < 0){
                            console.log("setLayout() :" + model.index + ": left_pos=" + left_pos + ", left is " + loader.parent)
                            loader.anchors.left = loader.parent.left
                            loader.anchors.leftMargin = 0
                        }else{
                            console.log("setLayout() :" + model.index + ": left_pos=" + left_pos + ", left name=" + repeater.itemAt(left_pos).item.settings.columnName)
                            loader.anchors.left = repeater.itemAt(left_pos).right
                            loader.anchors.leftMargin = 3
                        }
                    }

                    function setSettings() {
                        item.settings.autoLoading = model.autoLoading
                        item.settings.loadingInterval = model.loadingInterval
                        item.settings.columnName = model.name
                        item.settings.columnValue = model.value

                        item.settings.visibleLike = model.visibleLike
                        item.settings.visibleRepost = model.visibleRepost
                        item.settings.visibleFollow = model.visibleFollow
                        item.settings.visibleMention = model.visibleMention
                        item.settings.visibleReply = model.visibleReply
                        item.settings.visibleQuote = model.visibleQuote
                    }

                    function setAccount(row) {
                        item.account.service = accountListModel.item(row, AccountListModel.ServiceRole)
                        item.account.did = accountListModel.item(row, AccountListModel.DidRole)
                        item.account.handle = accountListModel.item(row, AccountListModel.HandleRole)
                        item.account.email = accountListModel.item(row, AccountListModel.EmailRole)
                        item.account.accessJwt = accountListModel.item(row, AccountListModel.AccessJwtRole)
                        item.account.refreshJwt = accountListModel.item(row, AccountListModel.RefreshJwtRole)
                        item.account.avatar = accountListModel.item(row, AccountListModel.AvatarRole)
                    }
                }
            }
        }
    }

    Frame {
        // ハイパーリンクの内容を表示する
        id: hoveredLinkFrame
        x: scrollView.x
        y: scrollView.height - scrollView.ScrollBar.horizontal.height - height
        visible: hoveredLinkFrame.text.length > 0
        leftInset: 5
        rightInset: 5
        topInset: 2
        bottomInset: 2
        background: Rectangle {
            radius: 3
            color: Material.color(Material.BlueGrey)
        }
        property string text: ""

        Label {
            color: "white"
            font.pointSize: 10
            text: hoveredLinkFrame.text
        }
    }

    ImageFullView {
        id: imageFullView
        anchors.fill: parent
        visible: false
    }
}
