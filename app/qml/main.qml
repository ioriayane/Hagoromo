import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import Qt.labs.settings 1.1

import tech.relog.hagoromo.accountlistmodel 1.0

import "dialogs"
import "view"

ApplicationWindow {
    width: 800
    height: 480
    visible: true
    title: qsTr("Hagromo")

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
            var component_type = "timeline"
            if(selectedTypeIndex == 0){
                component_type = "timeline"
            }else if(selectedTypeIndex == 1){
                component_type = "listNotification"
            }
            columnManageModel.appendColumn(selectedAccountIndex, component_type)
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
            for(var i=columnManageModel.count-1; i>=0; i--){
                exist = false
                for(var a=0; a<accountListModel.rowCount();a++){
                    if(columnManageModel.get(i).account_uuid === accountListModel.item(a, AccountListModel.UuidRole)){
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
    ListModel {
        id: columnManageModel
        ListElement {
            key: "abcdef"
            account_uuid: "{a51d2b54-2a28-40e2-b813-cf6a66f5027b}"
            account_index: 1
            component_type: "timeline"
        }
        ListElement {
            key: "ghijkl"
            account_uuid: "{56a9ca8d-c6e1-4a00-b2ba-15c569419882}"
            account_index: 0
            component_type: "listNotification"
        }

        function makeKey(){
            var key = ""
            for(var i=0; i<5; i++){
                key += Math.floor(Math.random() * 16).toString(16)
            }
            return key
        }

        function appendColumn(account_index, component_type) {
            columnManageModel.append({
                                         "key": makeKey(),
                                         "account_uuid": accountListModel.item(account_index, AccountListModel.UuidRole),
                                         "account_index": account_index,
                                         "component_type": component_type
                                     })
        }
        function removeColumnFromLoader(key){
            // カラム基準の操作で消すとき
            for(var i=columnManageModel.count-1; i>=0; i--){
                if(columnManageModel.get(i).key === key){
                    columnManageModel.remove(i)
                }
            }
        }
        function sync() {
            // 追加or更新
            for(var i=0; i<columnManageModel.count; i++){
                repeater.append(columnManageModel.get(i).key,
                                columnManageModel.get(i).account_uuid,
                                columnManageModel.get(i).account_index,
                                columnManageModel.get(i).component_type)
            }
            // カラムの管理情報から消えているLoaderを消す
            var exist = false
            for(var r=repeater.model.count-1; r>=0; r--){
                exist = false
                for(var c=0; c<columnManageModel.count; c++){
                    if(repeater.model.get(r).key === columnManageModel.get(c).key){
                        exist = true
                    }
                }
                if(exist === false){
                    repeater.model.remove(r)
                }
            }
        }
    }

    Component {
        id: columnView
        ColumnView {
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0
        Rectangle {
            Layout.fillHeight: true
            Layout.minimumWidth: 64
            Layout.maximumWidth: 128
            color: "#00000000"
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 1
                Button {
                    Layout.fillWidth: true
                    display: AbstractButton.IconOnly
                    // display: AbstractButton.TextBesideIcon
                    icon.source: "images/edit.png"
                    text: qsTr("New Post")
                    onClicked: postDialog.open()
                }

                Item {
                    Layout.preferredWidth: 1
                    Layout.fillHeight: true
                }

                Button {
                    Layout.fillWidth: true
                    display: AbstractButton.IconOnly
                    // display: AbstractButton.TextBesideIcon
                    icon.source: "images/column.png"
                    text: qsTr("Add column")

                    onClicked: addColumnDialog.open()
                }

                Button {
                    Layout.fillWidth: true
                    display: AbstractButton.IconOnly
                    // display: AbstractButton.TextBesideIcon
                    icon.source: "images/account.png"
                    text: qsTr("Account")

                    onClicked: accountDialog.open()
                }

                Button {
                    Layout.fillWidth: true
                    display: AbstractButton.IconOnly
                    // display: AbstractButton.TextBesideIcon
                    icon.source: "images/settings.png"
                    text: qsTr("Settings")

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

                    function append(key, account_uuid, account_index, component_type){
                        // accountListModelで管理するアカウントのindexと表示に使うコンポを指定
                        // ①ここでLoaderを追加する
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
                                                      "account_index": account_index,
                                                      "component_type": component_type
                                                  })
                        }
                    }
                    onItemAdded: (index, item) => {
                                     // ②Repeaterに追加されたLoaderにTLを表示するComponentを追加する
                                     //console.log("" + index + ":" + item + ":" + repeater.model.get(index).account_index)
                                     item.account_uuid = repeater.model.get(index).account_uuid
                                     item.account_index = repeater.model.get(index).account_index
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
                        property int account_index: -1
                        property string component_type: ""

                        onLoaded: {
                            // ③Loaderで読み込んだComponentにアカウント情報など設定する
                            console.log("loader:" + loader.account_index + ", " + loader.account_uuid)
                            if(loader.account_index < 0)
                                return
                            var i = loader.account_index
                            item.componentType = loader.component_type
                            item.service = accountListModel.item(i, AccountListModel.ServiceRole)
                            item.did = accountListModel.item(i, AccountListModel.DidRole)
                            item.handle = accountListModel.item(i, AccountListModel.HandleRole)
                            item.email = accountListModel.item(i, AccountListModel.EmailRole)
                            item.accessJwt = accountListModel.item(i, AccountListModel.AccessJwtRole)
                            item.refreshJwt = accountListModel.item(i, AccountListModel.RefreshJwtRole)
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
                            text: model.account_uuid
                        }
                        Label {
                            text: "account_index"
                        }
                        Label {
                            text: model.account_index
                        }
                        Label {
                            text: "component_type"
                        }
                        Label {
                            text: model.component_type
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
