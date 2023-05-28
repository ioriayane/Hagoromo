import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import Qt.labs.settings 1.1

import tech.relog.hagoromo.accountlistmodel 1.0
import tech.relog.hagoromo.columnlistmodel 1.0

import "controls"
import "dialogs"
import "view"

ApplicationWindow {
    id: appWindow
    width: 800
    height: 480
    visible: true
    title: qsTr("Hagoromo")

    Material.theme: settingDialog.settings.theme
    Material.accent: settingDialog.settings.accent

    Shortcut {  // Post
        enabled: !postDialog.visible
        context: Qt.ApplicationShortcut
        sequence: "n"
        onActivated: postDialog.open()
    }

    SettingDialog {
        id: settingDialog
    }

    PostDialog {
        id: postDialog
        accountModel: accountListModel
    }

    AddColumnDialog {
        id: addColumnDialog
        accountModel: accountListModel
        onAccepted: {
            console.log("selectedAccountIndex=" + selectedAccountIndex + ", selectedTypeIndex=" + selectedTypeIndex)
            var component_type = 0
            if(selectedTypeIndex == 0){
                component_type = 0
            }else if(selectedTypeIndex == 1){
                component_type = 1
            }
            columnManageModel.append(accountListModel.item(selectedAccountIndex, AccountListModel.UuidRole),
                                     component_type, false, 300000, 400)
            columnManageModel.sync()
        }
    }

    AccountDialog {
        id: accountDialog
        accountModel: accountListModel
        onClosed: accountListModel.syncColumn()
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
                               console.log("onAppendedAccount:" + row)
                           }
        onUpdatedAccount: (row, uuid) => {
                              console.log("onUpdatedAccount:" + row + ", " + uuid)
                              // カラムを更新しにいく
                              repeater.updateAccount(uuid)
                          }

        onAllFinished: {
            // すべてのアカウント情報の認証が終わったのでカラムを復元（成功しているとは限らない）
            console.log("allFinished()" + accountListModel.rowCount())
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
                                columnManageModel.item(i, ColumnListModel.WidthRole)
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
            console.log("exchange:" + key + ", " + i + ", " + move_diff)
            var account_uuid = columnManageModel.item(i, ColumnListModel.AccountUuidRole)
            var component_type = columnManageModel.item(i, ColumnListModel.ComponentTypeRole)
            var auto_loading = columnManageModel.item(i, ColumnListModel.AutoLoadingRole)
            var loading_interval = columnManageModel.item(i, ColumnListModel.LoadingIntervalRole)
            var column_width = columnManageModel.item(i, ColumnListModel.WidthRole)
            // 1度消す
            columnManageModel.remove(i)
            columnManageModel.sync()
            // 追加し直し
            columnManageModel.insert(i+move_diff, account_uuid, component_type,
                                     auto_loading, loading_interval, column_width)
            columnManageModel.sync()
        }
    }

    Component {
        id: columnView
        ColumnView {
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

            onRequestViewImages: (index, paths) => imageFullView.open(index, paths)

            onRequestMoveToLeft: (key) => {
                                       console.log("move to left:" + key)
                                       columnManageModel.exchange(key, -1)
                                   }
            onRequestMoveToRight: (key) => {
                                        console.log("move to right:" + key)
                                        columnManageModel.exchange(key, 1)
                                    }
            onRequestRemove: (key) => {
                                   console.log("remove column:" + key)
                                   columnManageModel.removeByKey(key)
                                   columnManageModel.sync()
                               }
            onRequestDisplayOfColumnSetting: (key) => columnsettingDialog.openWithKey(key)
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
                                    console.log("Update column : col=" + i + ", a_row=" + row)
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

                    function insert(index, key, account_uuid, component_type, auto_loading, loading_interval, column_width){
                        // accountListModelで管理するアカウントのindexと表示に使うコンポを指定
                        // ①ここでLoaderを追加する
                        console.log("(1) insert:" + index + ", " + account_uuid + ", " + component_type)
                        if(contains(key)){
                            // 既にある
                            var item = repeater.itemAt(index)   //ここのitemはloader自身
                            if(item.key === key){
                                console.log("  update only:" + auto_loading + ", " + loading_interval + ", " + column_width)
                                item.auto_loading = auto_loading
                                item.loading_interval = loading_interval
                                item.Layout.preferredWidth = column_width
                                item.setSettings()
                            }
                        }else{
                            repeater.model.insert(index, {
                                                      "key": key,
                                                      "account_uuid": account_uuid,
                                                      "component_type": component_type,
                                                      "auto_loading": auto_loading,
                                                      "loading_interval": loading_interval,
                                                      "column_width": column_width
                                                  })
                        }
                    }
                    onItemAdded: (index, item) => {
                                     // ②Repeaterに追加されたLoaderにTLを表示するComponentを追加する
                                     console.log("(2) onItemAdded:" + index + ":" + item)
                                     item.key = repeater.model.get(index).key
                                     item.account_uuid = repeater.model.get(index).account_uuid
                                     item.component_type = repeater.model.get(index).component_type
                                     item.auto_loading = repeater.model.get(index).auto_loading
                                     item.loading_interval = repeater.model.get(index).loading_interval
                                     item.column_width = repeater.model.get(index).column_width
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

                        onLoaded: {
                            // ③Loaderで読み込んだComponentにアカウント情報など設定する
                            var row = accountListModel.indexAt(loader.account_uuid)
                            console.log("(3) loader:" + row + ", " + loader.account_uuid)
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

                // debug
                ListView {
                    Layout.preferredHeight: scrollView.childHeight
                    Layout.minimumWidth: 100
                    Layout.preferredWidth: 400
                    Layout.maximumWidth: 500
                    clip: true
                    model: accountListModel
                    delegate: GridLayout {
                        columns: 2
                        Label {
                            text: "service:"
                        }
                        Label {
                            text: model.service
                        }
                        Label {
                            text: "identifier:"
                        }
                        Label {
                            text: model.identifier
                        }
                        Label {
                            text: "password:"
                        }
                        Label {
                            text: model.password
                        }
                        Label {
                            text: "did:"
                        }
                        Label {
                            text: model.did
                        }
                        Label {
                            text: "handle:"
                        }
                        Label {
                            text: model.handle
                        }
                        Label {
                            text: "email:"
                        }
                        Label {
                            text: model.email
                        }
                        Label {
                            text: "accessJwt:"
                        }
                        Label {
                            text: model.accessJwt
                        }
                        Label {
                            text: "refreshJwt:"
                        }
                        Label {
                            text: model.refreshJwt
                        }
                        Label {
                            text: "status:"
                        }
                        Label {
                            text: model.status
                        }
                        Label {
                            text: "-"
                        }
                        Label {
                            text: "-"
                        }

                    }
                }
                ListView {
                    Layout.preferredHeight: scrollView.childHeight
                    Layout.minimumWidth: 100
                    Layout.preferredWidth: 200
                    Layout.maximumWidth: 300
                    clip: true
                    model: columnManageModel
                    delegate: GridLayout {
                        columns: 2
                        Label {
                            text: "key"
                        }
                        Label {
                            text: model.key
                        }
                        Label {
                            text: "account_uuid"
                        }
                        Label {
                            text: model.accountUuid
                        }
                        Label {
                            text: "component_type"
                        }
                        Label {
                            text: model.componentType
                        }
                        Label {
                            text: "autoLoading"
                        }
                        Label {
                            text: model.autoLoading
                        }
                        Label {
                            text: "interval"
                        }
                        Label {
                            text: model.loadingInterval
                        }
                        Label {
                            text: "width"
                        }
                        Label {
                            text: model.width
                        }
                        Label {
                            text: "-"
                        }
                        Label {
                            text: "-"
                        }
                    }
                }
            }
        }
    }

    ImageFullView {
        id: imageFullView
        anchors.fill: parent
        visible: false
    }
}
