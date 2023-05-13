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

    Material.theme: Material.Dark

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
                                     component_type)
            columnManageModel.sync()
        }
    }

    AccountDialog {
        id: accountDialog
        accountModel: accountListModel
        onClosed: accountListModel.syncColumn()
    }

    // アカウントの管理
    // カラムとこのモデルとの紐付けはインデックスで実施する
    // アカウント管理で内容が変更されたときにカラムとインデックスの関係が崩れるのでuuidで確認する
    AccountListModel {
        id: accountListModel
        onAccountAppended: (row) => {
                               console.log("accountAppended:" + row)
                               // カラムを更新しにいく
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
                repeater.append(columnManageModel.item(i, ColumnListModel.KeyRole),
                                columnManageModel.item(i, ColumnListModel.AccountUuidRole),
                                columnManageModel.item(i, ColumnListModel.ComponentTypeRole))
            }
            // カラムの管理情報から消えているLoaderを消す
            for(var r=repeater.model.count-1; r>=0; r--){
                if(columnManageModel.containsKey(repeater.model.get(r).key) === false){
                    repeater.model.remove(r)
                }
            }
        }
    }

    Component {
        id: columnView
        ColumnView {
            onRequestedReply: (account_uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) => {
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
            onRequestedQuote: (account_uuid, cid, uri, avatar, display_name, handle, indexed_at, text) => {
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

                    function append(key, account_uuid, component_type){
                        // accountListModelで管理するアカウントのindexと表示に使うコンポを指定
                        // ①ここでLoaderを追加する
                        console.log("(1) append:" + account_uuid + ", " + component_type)
                        var exist = false
                        for(var i=0; i<repeater.model.count; i++){
                            if(repeater.model.get(i).key === key){
                                exist = true
                                break
                            }
                        }

                        if(exist){
                            // 既にある
                        }else{
                            repeater.model.append({
                                                      "key": key,
                                                      "account_uuid": account_uuid,
                                                      "component_type": component_type
                                                  })
                        }
                    }
                    onItemAdded: (index, item) => {
                                     // ②Repeaterに追加されたLoaderにTLを表示するComponentを追加する
                                     console.log("(2) onItemAdded:" + index + ":" + item)
                                     item.account_uuid = repeater.model.get(index).account_uuid
                                     item.component_type = repeater.model.get(index).component_type
                                     item.sourceComponent = columnView
                                 }

                    Loader {
                        id: loader
                        Layout.preferredHeight: scrollView.childHeight
                        Layout.minimumWidth: 100
                        Layout.preferredWidth: 400
                        Layout.maximumWidth: 500

                        property string account_uuid: ""
                        property int component_type: 0

                        onLoaded: {
                            // ③Loaderで読み込んだComponentにアカウント情報など設定する
                            var i = accountListModel.indexAt(loader.account_uuid)
                            console.log("(3) loader:" + i + ", " + loader.account_uuid)
                            if(i < 0)
                                return
                            item.componentType = loader.component_type
                            item.accountUuid = loader.account_uuid
                            item.service = accountListModel.item(i, AccountListModel.ServiceRole)
                            item.did = accountListModel.item(i, AccountListModel.DidRole)
                            item.handle = accountListModel.item(i, AccountListModel.HandleRole)
                            item.email = accountListModel.item(i, AccountListModel.EmailRole)
                            item.accessJwt = accountListModel.item(i, AccountListModel.AccessJwtRole)
                            item.refreshJwt = accountListModel.item(i, AccountListModel.RefreshJwtRole)
                            item.rootItem = rootLayout
                            item.load()
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
}
