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
            columnManageModel.sync()
        }
    }

    AddColumnDialog {
        id: addColumnDialog
        accountModel: accountListModel
        onAccepted: {
            console.log(/*logMain,*/ "Add column\n  selectedAccountIndex=" + selectedAccountIndex +
                        "\n  selectedType=" + selectedType +
                        "\n  selectedName=" + selectedName +
                        "\n  selectedUri=" + selectedUri)
            columnManageModel.append(accountListModel.item(selectedAccountIndex, AccountListModel.UuidRole),
                                     selectedType, false, 300000, 400, selectedName, selectedUri)
            columnManageModel.sync()
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
            columnManageModel.sync()
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

                columnManageModel.sync()
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
            columnManageModel.sync()
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
            columnManageModel.sync()
        }
    }


    //カラムの情報管理
    ColumnListModel {
        id: columnManageModel

        function sync() {
            // 追加or更新
            for(var i=0; i<columnManageModel.rowCount(); i++){
                repeater.insert(i,
                                columnManageModel.item(i, ColumnListModel.KeyRole),
                                columnManageModel.item(i, ColumnListModel.AccountUuidRole),
                                columnManageModel.item(i, ColumnListModel.ComponentTypeRole),
                                columnManageModel.item(i, ColumnListModel.AutoLoadingRole),
                                columnManageModel.item(i, ColumnListModel.LoadingIntervalRole),
                                columnManageModel.item(i, ColumnListModel.WidthRole),
                                columnManageModel.item(i, ColumnListModel.NameRole),
                                columnManageModel.item(i, ColumnListModel.ValueRole)
                                )
            }
            // カラムの管理情報から消えているLoaderを消す
            for(var r=repeater.model.count-1; r>=0; r--){
                if(columnManageModel.containsKey(repeater.model.get(r).key) === false){
                    repeater.model.remove(r)
                }
            }
        }
        function exchange(key, move_diff) {
            // move_diff : -1=left, 1=right
            var i = columnManageModel.indexOf(key)
            console.log(logMain, "exchange:" + key + ", " + i + ", " + move_diff)
            var account_uuid = columnManageModel.item(i, ColumnListModel.AccountUuidRole)
            var component_type = columnManageModel.item(i, ColumnListModel.ComponentTypeRole)
            var auto_loading = columnManageModel.item(i, ColumnListModel.AutoLoadingRole)
            var loading_interval = columnManageModel.item(i, ColumnListModel.LoadingIntervalRole)
            var column_width = columnManageModel.item(i, ColumnListModel.WidthRole)
            var column_name = columnManageModel.item(i, ColumnListModel.NameRole)
            var column_value = columnManageModel.item(i, ColumnListModel.ValueRole)
            // 1度消す
            columnManageModel.remove(i)
            columnManageModel.sync()
            // 追加し直し
            columnManageModel.insert(i+move_diff, account_uuid, component_type,
                                     auto_loading, loading_interval, column_width,
                                     column_name, column_value)
            columnManageModel.sync()
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
                                     columnManageModel.exchange(key, -1)
                                 }
            onRequestMoveToRight: (key) => {
                                      console.log(logMain, "move to right:" + key)
                                      columnManageModel.exchange(key, 1)
                                  }
            onRequestRemove: (key) => {
                                 console.log(logMain, "remove column:" + key)
                                 columnManageModel.removeByKey(key)
                                 columnManageModel.sync()
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

            RowLayout {
                spacing: 3
                Repeater {
                    id: repeater
                    model: ListModel {}

                    function updateAccount(account_uuid){
                        for(var i=0; i<repeater.count; i++){
                            var item = repeater.itemAt(i)   //ここのitemはloader自身
                            if(item.account_uuid === account_uuid){
                                var row = accountListModel.indexAt(item.account_uuid)
                                if(row >= 0){
                                    console.log(logMain, "Update column : col=" + i + ", a_row=" + row)
                                    item.setAccount(row)
                                    item.item.reflect()
                                }
                            }
                        }
                    }

                    function contains(key){
                        for(var i=0; i<repeater.model.count; i++){
                            if(repeater.model.get(i).key === key){
                                return true
                            }
                        }
                        return false
                    }

                    function insert(index, key, account_uuid, component_type,
                                    auto_loading, loading_interval, column_width,
                                    column_name, column_value){
                        // accountListModelで管理するアカウントのindexと表示に使うコンポを指定
                        // ①ここでLoaderを追加する
                        console.log(logMain, "(1) insert:" + index + ", " + account_uuid + ", " + component_type)
                        if(contains(key)){
                            // 既にある
                            var item = repeater.itemAt(index)   //ここのitemはloader自身
                            if(item.key === key){
                                console.log(logMain, "  update only:" + auto_loading + ", " + loading_interval + ", " + column_width)
                                item.auto_loading = auto_loading
                                item.loading_interval = loading_interval
                                item.Layout.preferredWidth = column_width
                                item.column_name = column_name
                                item.column_value = column_value
                                item.setSettings()
                            }
                        }else{
                            repeater.model.insert(index, {
                                                      "key": key,
                                                      "account_uuid": account_uuid,
                                                      "component_type": component_type,
                                                      "auto_loading": auto_loading,
                                                      "loading_interval": loading_interval,
                                                      "column_width": column_width,
                                                      "column_name": column_name,
                                                      "column_value": column_value
                                                  })
                        }
                    }
                    onItemAdded: (index, item) => {
                                     // ②Repeaterに追加されたLoaderにTLを表示するComponentを追加する
                                     console.log(logMain, "(2) onItemAdded:" + index + ":" + item)
                                     item.key = repeater.model.get(index).key
                                     item.account_uuid = repeater.model.get(index).account_uuid
                                     item.component_type = repeater.model.get(index).component_type
                                     item.auto_loading = repeater.model.get(index).auto_loading
                                     item.loading_interval = repeater.model.get(index).loading_interval
                                     item.column_width = repeater.model.get(index).column_width
                                     item.column_name = repeater.model.get(index).column_name
                                     item.column_value = repeater.model.get(index).column_value
                                     item.sourceComponent = columnView
                                 }

                    Loader {
                        id: loader
                        Layout.preferredHeight: scrollView.childHeight
                        Layout.minimumWidth: 100
                        Layout.preferredWidth: column_width
                        Layout.maximumWidth: 500

                        property string key: ""
                        property string account_uuid: ""
                        property int component_type: 0
                        property bool auto_loading: false
                        property int loading_interval: 300000
                        property int column_width: 400
                        property string column_name: ""
                        property string column_value: ""

                        onLoaded: {
                            // ③Loaderで読み込んだComponentにアカウント情報など設定する
                            var row = accountListModel.indexAt(loader.account_uuid)
                            console.log(logMain, "(3) loader:" + row + ", " + loader.account_uuid)
                            if(row < 0)
                                return
                            item.columnKey = loader.key
                            item.componentType = loader.component_type
                            item.accountUuid = loader.account_uuid
                            setSettings()
                            setAccount(row)
                            item.rootItem = rootLayout
                            item.load()
                        }

                        function setSettings() {
                            item.autoLoading = loader.auto_loading
                            item.loadingInterval = loader.loading_interval
                            item.columnName = loader.column_name
                            item.columnValue = loader.column_value
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
