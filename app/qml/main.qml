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

    LoggingCategory {
        id: logMain
        name: "tech.relog.hagoromo.Main"
        defaultLogLevel: LoggingCategory.Warning
    }

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
            console.log(logMain, "selectedAccountIndex=" + selectedAccountIndex + ", searchType=" + searchType)
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
            console.log(logMain, "Add column\n  selectedAccountIndex=" + selectedAccountIndex +
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
            console.log(logMain, "open column setting dialog:" + i + ", " + columnKey)
            if(i >= 0){
                autoLoadingCheckbox.checked = columnManageModel.item(i, ColumnListModel.AutoLoadingRole)
                autoLoadingIntervalCombo.setByValue(columnManageModel.item(i, ColumnListModel.LoadingIntervalRole))
                columnWidthSlider.value = columnManageModel.item(i, ColumnListModel.WidthRole)

                open()
            }
        }

        onAccepted: {
            var i = columnManageModel.indexOf(columnKey)
            if(i >= 0){
                columnManageModel.update(i, ColumnListModel.AutoLoadingRole, autoLoadingCheckbox.checked)
                columnManageModel.update(i, ColumnListModel.LoadingIntervalRole, autoLoadingIntervalCombo.currentValue)
                columnManageModel.update(i, ColumnListModel.WidthRole, columnWidthSlider.value)

                repeater.updateSetting()
            }
        }
    }

    // アカウントの管理
    // カラムとこのモデルとの紐付けはインデックスで実施する
    // アカウント管理で内容が変更されたときにカラムとインデックスの関係が崩れるのでuuidで確認する
    AccountListModel {
        id: accountListModel
        onAppendedAccount: (row) => {
                               console.log(logMain, "onAppendedAccount:" + row)
                           }
        onUpdatedAccount: (row, uuid) => {
                              console.log(logMain, "onUpdatedAccount:" + row + ", " + uuid)
                              // カラムを更新しにいく
                              repeater.updateAccount(uuid)
                          }

        onAllFinished: {
            // すべてのアカウント情報の認証が終わったのでカラムを復元（成功しているとは限らない）
            console.log(logMain, "allFinished()" + accountListModel.rowCount())
            if(columnManageModel.rowCount() === 0){
                columnManageModel.load()
            }
        }

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
            onRequestReply: (account_uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) => {
                                console.log(logMain,
                                            account_uuid + ",\n" +
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

            onRequestViewImages: (index, paths) => imageFullView.open(index, paths)

            onRequestMoveToLeft: (key) => {
                                     console.log(logMain, "move to left:" + key)
                                     columnManageModel.move(key, ColumnListModel.MoveLeft)
                                     repeater.updatePosition()
                                 }
            onRequestMoveToRight: (key) => {
                                      console.log(logMain, "move to right:" + key)
                                      columnManageModel.move(key, ColumnListModel.MoveRight)
                                      repeater.updatePosition()
                                  }
            onRequestRemove: (key) => {
                                 console.log(logMain, "remove column:" + key)
                                 columnManageModel.removeByKey(key)
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
                    display: AbstractButton.IconOnly
                    iconSource: "images/edit.png"
                    //                    iconText: qsTr("New Post")
                    onClicked: postDialog.open()
                }

                IconButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
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

            Repeater {
                id: repeater
                model: ColumnListModel {
                    //カラムの情報管理
                    id: columnManageModel
                }

                function updateSetting() {
                    for(var i=0; i<repeater.count; i++){
                        var item = repeater.itemAt(i)   //ここのitemはloader自身
                        item.setSettings()
                    }
                }

                function updatePosition() {
                    var i;
                    for(i=0; i<repeater.count; i++){
                        repeater.itemAt(i).item.anchors.left = undefined
                        //ここのitemはloader自身
                    }
                    for(i=0; i<repeater.count; i++){
                        var item = repeater.itemAt(i)   //ここのitemはloader自身
                        var left_pos = columnManageModel.getLeftPosition(i)
                        console.log("updatePosition : i=" + i + ", leftPost=" + left_pos)
                        if(left_pos < 0){
                            item.anchors.left = item.parent.left
                        }else{
                            item.anchors.left = repeater.itemAt(left_pos).right
                        }
                    }
                }

                function updateAccount(account_uuid){
                    for(var i=0; i<repeater.count; i++){
                        var item = repeater.itemAt(i)   //ここのitemはloader自身
                        if(item.item.accountUuid === account_uuid){
                            var row = accountListModel.indexAt(item.item.accountUuid)
                            if(row >= 0){
                                console.log(logMain, "Update column : col=" + i + ", a_row=" + row)
                                item.setAccount(row)
                                item.item.reflect()
                            }
                        }
                    }
                }

                onItemAdded: (index, item) => {
                                 // ②Repeaterに追加されたLoaderのレイアウト設定
                                 var left_pos = columnManageModel.getLeftPosition(index)
                                 if(left_pos < 0){
                                     item.anchors.left = parent.left
                                 }else{
                                     item.anchors.left = repeater.itemAt(left_pos).right
                                 }
                             }

                Loader {
                    id: loader
                    height: scrollView.childHeight
                    width: model.width
                    sourceComponent: columnView

                    onLoaded: {
                        // ③Loaderで読み込んだComponentにアカウント情報など設定する
                        var row = accountListModel.indexAt(model.accountUuid)
                        console.log(/*logMain,*/ "(3) loader:" + row + ", " + model.accountUuid)
                        if(row < 0){
                            return
                        }

                        item.columnKey = model.key
                        item.componentType = model.componentType
                        item.accountUuid = model.accountUuid
                        setSettings()
                        setAccount(row)
                        item.rootItem = rootLayout
                        item.load()
                    }

                    function setSettings() {
                        item.autoLoading = model.autoLoading
                        item.loadingInterval = model.loadingInterval
                        item.columnName = model.name
                        item.columnValue = model.value
                    }

                    function setAccount(row) {
                        item.service = accountListModel.item(row, AccountListModel.ServiceRole)
                        item.did = accountListModel.item(row, AccountListModel.DidRole)
                        item.handle = accountListModel.item(row, AccountListModel.HandleRole)
                        item.email = accountListModel.item(row, AccountListModel.EmailRole)
                        item.accessJwt = accountListModel.item(row, AccountListModel.AccessJwtRole)
                        item.refreshJwt = accountListModel.item(row, AccountListModel.RefreshJwtRole)
                        item.avatar = accountListModel.item(row, AccountListModel.AvatarRole)
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
