import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15
import Qt.labs.settings 1.1

import tech.relog.hagoromo.accountlistmodel 1.0
import tech.relog.hagoromo.columnlistmodel 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "controls"
import "dialogs"
import "view"
import "parts"

ApplicationWindow {
    id: appWindow
    width: 800
    height: 600
    minimumWidth: 800
    minimumHeight: 600 * AdjustedValues.ratioHalf
    visible: true
    title: "羽衣 -Hagoromo-"

    Material.theme: settingDialog.settings.theme
    Material.accent: settingDialog.settings.accent
    // SystemTools::updateFont()で裏からすべてのフォントを変更出来れば良いが
    // Componentの内容を動的に読み込むケースに対応するため、ここを使用する
    // 逆にこの設定はListViewの内容へ反映されない
    font.family: settingDialog.settings.fontFamily.length > 0 ? settingDialog.settings.fontFamily : font.family

    function errorHandler(account_uuid, code, message) {
        if(code === "ExpiredToken" && account_uuid.length > 0){
            accountListModel.refreshAccountSession(account_uuid)
        }else if(message.length === 0){
        }else{
            var row = accountListModel.indexAt(account_uuid)
            var handle = ""
            if(row >= 0){
                handle = accountListModel.item(row, AccountListModel.HandleRole)
            }
            console.log("ERROR: " + handle + "(" + account_uuid + ") " + code + ":" + message)
            message += "\n\n@" + handle
            messageDialog.show("error", code, message)
        }
    }

    Settings {
        //        property alias x: appWindow.x
        //        property alias y: appWindow.y
        property alias width: appWindow.width
        property alias height: appWindow.height
    }

    SystemTool {
        id: systemTool
    }

    ApplicationShortcut {
        enabled: !postDialog.visible && !searchDialog.visible
        postDialogShortcut.onActivated: postDialog.open()
        searchDialogShortcut.onActivated: searchDialog.open()
        onShowColumn: (index) => {
                          if(index === -1){
                              //一番右
                              scrollView.showRightMost()
                          }else if(index === 1){
                              scrollView.showLeftMost()
                          }else if(index > 1 && index <= 9){
                              scrollView.showColumn(index-1)
                          }
                      }
    }


    SettingDialog {
        id: settingDialog
        onAccepted: repeater.updateSetting()
    }

    PostDialog {
        id: postDialog
        accountModel: accountListModel
        onErrorOccured: (account_uuid, code, message) => appWindow.errorHandler(account_uuid, code, message)
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
            scrollView.showRightMost()
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
            scrollView.showRightMost()
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
        onErrorOccured: (account_uuid, code, message) => appWindow.errorHandler(account_uuid, code, message)
    }

    AccountDialog {
        id: accountDialog
        accountModel: accountListModel
        onClosed: accountListModel.syncColumn()
        onErrorOccured: (account_uuid, code, message) => appWindow.errorHandler(account_uuid, code, message)
    }

    DiscoverFeedsDialog {
        id: discoverFeedsDialog
        onAccepted: {
            columnManageModel.append(discoverFeedsDialog.account.uuid,
                                     4, false, 300000, 400, selectedName, selectedUri)
            scrollView.showRightMost()
        }
        onErrorOccured: (account_uuid, code, message) => appWindow.errorHandler(account_uuid, code, message)
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
                visibleReplyToUnfollowedUsersCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleReplyToUnfollowedUsersRole)

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
                columnManageModel.update(i, ColumnListModel.VisibleReplyToUnfollowedUsersRole, visibleReplyToUnfollowedUsersCheckBox.checked)

                repeater.updateSetting()
            }
        }
    }

    ReportPostDialog {
        id: reportDialog
        onErrorOccured: (account_uuid, code, message) => appWindow.errorHandler(account_uuid, code, message)
    }
    ReportAccountDialog {
        id: reportAccountDialog
        onErrorOccured: (account_uuid, code, message) => appWindow.errorHandler(account_uuid, code, message)
    }
    AddToListDialog {
        id: addToListDialog
        onErrorOccured: (account_uuid, code, message) => appWindow.errorHandler(account_uuid, code, message)
    }
    AddListDialog {
        id: addListDialog
        onErrorOccured: (account_uuid, code, message) => appWindow.errorHandler(account_uuid, code, message)
    }
    EditProfileDialog {
        id: editProfileDialog
        onErrorOccured: (account_uuid, code, message) => appWindow.errorHandler(account_uuid, code, message)
        onAccepted: accountListModel.refreshAccountProfile(editProfileDialog.account.uuid)
    }

    MessageDialog {
        id: messageDialog
    }

    // アカウントの管理
    // カラムとこのモデルとの紐付けはインデックスで実施する
    // アカウント管理で内容が変更されたときにカラムとインデックスの関係が崩れるのでuuidで確認する
    AccountListModel {
        id: accountListModel
        onUpdatedSession: (row, uuid) => {
                              console.log("onUpdatedSession:" + row + ", " + uuid)
                              if(columnManageModel.rowCount() === 0 && allAccountsReady()){
                                  // すべてのアカウント情報の認証が終わったのでカラムを復元
                                  console.log("start loading columns")
                                  columnManageModel.load()
                              }
                          }
        onUpdatedAccount: (row, uuid) => {
                              console.log("onUpdatedAccount:" + row + ", " + uuid)
                              // カラムを更新しにいく
                              repeater.updateAccount(uuid)
                          }
        onErrorOccured: (code, message) => appWindow.errorHandler("", code, message)

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
                                         scrollView.showRightMost()
                                     }
            onRequestViewImages: (index, paths, alts) => imageFullView.open(index, paths, alts)
            onRequestViewFeedGenerator: (account_uuid, name, uri) => {
                                            columnManageModel.append(account_uuid, 4, false, 300000, 400, name, uri)
                                            scrollView.showRightMost()
                                        }
            onRequestViewSearchPosts: (account_uuid, text, current_column_key) => {
                                          console.log("Search:" + account_uuid + ", " + text + ", " + current_column_key)
                                          var pos = columnManageModel.insertNext(current_column_key, account_uuid, 2, false, 300000, 350,
                                                                                 qsTr("Search posts"), text)
                                          repeater.updatePosition()
                                          scrollView.showColumn(pos)
                                      }
            onRequestViewListFeed: (account_uuid, uri, name) => {
                                       console.log("uuid=" + account_uuid + "\nuri=" + uri + "\nname=" + name)
                                       columnManageModel.append(account_uuid, 6, false, 300000, 400, name, uri)
                                       scrollView.showRightMost()
                                   }

            onRequestReportPost: (account_uuid, uri, cid) => {
                                     var row = accountListModel.indexAt(account_uuid)
                                     if(row >= 0){
                                         reportDialog.targetUri = uri
                                         reportDialog.targetCid = cid
                                         reportDialog.account.uuid = account_uuid
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
                                            reportAccountDialog.account.uuid = account_uuid
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
            onRequestAddRemoveFromLists: (account_uuid, did) => {
                                             var row = accountListModel.indexAt(account_uuid)
                                             if(row >= 0){
                                                 addToListDialog.targetDid = did
                                                 addToListDialog.account.uuid = account_uuid
                                                 addToListDialog.account.service = accountListModel.item(row, AccountListModel.ServiceRole)
                                                 addToListDialog.account.did = accountListModel.item(row, AccountListModel.DidRole)
                                                 addToListDialog.account.handle = accountListModel.item(row, AccountListModel.HandleRole)
                                                 addToListDialog.account.email = accountListModel.item(row, AccountListModel.EmailRole)
                                                 addToListDialog.account.accessJwt = accountListModel.item(row, AccountListModel.AccessJwtRole)
                                                 addToListDialog.account.refreshJwt = accountListModel.item(row, AccountListModel.RefreshJwtRole)
                                                 addToListDialog.account.avatar = accountListModel.item(row, AccountListModel.AvatarRole)
                                                 addToListDialog.open()
                                             }
                                         }
            onRequestEditProfile: (account_uuid, did, avatar, banner, display_name, description) => {
                                      var row = accountListModel.indexAt(account_uuid)
                                      if(row >= 0){
                                          editProfileDialog.avatar = avatar
                                          editProfileDialog.banner = banner
                                          editProfileDialog.displayName = display_name
                                          editProfileDialog.description = description
                                          editProfileDialog.account.uuid = account_uuid
                                          editProfileDialog.account.service = accountListModel.item(row, AccountListModel.ServiceRole)
                                          editProfileDialog.account.did = accountListModel.item(row, AccountListModel.DidRole)
                                          editProfileDialog.account.handle = accountListModel.item(row, AccountListModel.HandleRole)
                                          editProfileDialog.account.email = accountListModel.item(row, AccountListModel.EmailRole)
                                          editProfileDialog.account.accessJwt = accountListModel.item(row, AccountListModel.AccessJwtRole)
                                          editProfileDialog.account.refreshJwt = accountListModel.item(row, AccountListModel.RefreshJwtRole)
                                          editProfileDialog.account.avatar = accountListModel.item(row, AccountListModel.AvatarRole)
                                          editProfileDialog.open()
                                      }
                                  }
            onRequestEditList: (account_uuid, uri, avatar, name, description) => {
                                   var row = accountListModel.indexAt(account_uuid)
                                   if(row >= 0){
                                       addListDialog.editMode = true
                                       addListDialog.listUri = uri
                                       addListDialog.avatar = avatar
                                       addListDialog.displayName = name
                                       addListDialog.description = description
                                       addListDialog.account.uuid = account_uuid
                                       addListDialog.account.service = accountListModel.item(row, AccountListModel.ServiceRole)
                                       addListDialog.account.did = accountListModel.item(row, AccountListModel.DidRole)
                                       addListDialog.account.handle = accountListModel.item(row, AccountListModel.HandleRole)
                                       addListDialog.account.email = accountListModel.item(row, AccountListModel.EmailRole)
                                       addListDialog.account.accessJwt = accountListModel.item(row, AccountListModel.AccessJwtRole)
                                       addListDialog.account.refreshJwt = accountListModel.item(row, AccountListModel.RefreshJwtRole)
                                       addListDialog.account.avatar = accountListModel.item(row, AccountListModel.AvatarRole)
                                       addListDialog.open()
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
            onErrorOccured: (account_uuid, code, message) => appWindow.errorHandler(account_uuid, code, message)
        }
    }

    VersionInfomation {
        anchors.right: rootLayout.right
        anchors.bottom: rootLayout.bottom
        anchors.rightMargin: 5
        anchors.bottomMargin: scrollView.ScrollBar.horizontal.height + 5
        visible: ((scrollView.contentWidth + sideBarItem.width) < x) && settingDialog.settings.displayVersionInfoInMainArea
    }

    RowLayout {
        id: rootLayout
        anchors.fill: parent
        spacing: 0
        Item {
            id: sideBarItem
            Layout.fillHeight: true
            Layout.minimumWidth: 48
            Layout.preferredWidth: AdjustedValues.v48
            Layout.maximumWidth: AdjustedValues.v96
            SideBar {
                anchors.fill: parent
                anchors.margins: 1
                post.onClicked: postDialog.open()
                search.onClicked: searchDialog.open()
                addColumn.onClicked: addColumnDialog.open()
                moderation.onClicked: console.log("click moderation")
                account.onClicked: accountDialog.open()
                setting.onClicked: settingDialog.open()
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

            function showLeftMost() {
                // scrollView.contentItem.contentX = 0
                showColumn(0)
            }

            function showRightMost() {
                // scrollView.contentItemはFlickable
                // Flickableの幅が全カラムの幅より小さくなったら移動させる
                //if(scrollView.contentWidth > scrollView.contentItem.width){
                //    scrollView.contentItem.contentX = scrollView.contentWidth - scrollView.contentItem.width
                //}
                showColumn(repeater.count - 1)
            }

            function showColumn(index){
                var w = 0
                var last_w = 0
                var margin = 0
                var row_list = columnManageModel.getRowListInOrderOfPosition()
                var item = undefined
                if(row_list.length <= index){
                    return
                }
                for(var i=0; i<row_list.length; i++){
                    if(i < index){
                        item = repeater.itemAt(row_list[i])   //ここのitemはloader自身
                        w += item.width + item.anchors.leftMargin
                    }else if(i === index){
                        item = repeater.itemAt(row_list[i])   //ここのitemはloader自身
                        last_w = item.width + item.anchors.leftMargin
                        margin = item.anchors.leftMargin
                    }
                }
                if((w + last_w) > scrollView.contentItem.width){
                    scrollView.contentItem.contentX = (w + last_w) - scrollView.contentItem.width
                }else if(w < scrollView.contentItem.contentX){
                    scrollView.contentItem.contentX = w
                }

                columnCursor.width = last_w - margin
                columnCursor.height = scrollView.height - scrollView.ScrollBar.horizontal.height - 1
                columnCursor.x = scrollView.x + w + margin - scrollView.contentItem.contentX
                columnCursor.y = 1
                columnCursorAnimation.stop()
                columnCursorAnimation.start()
            }

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
                    width: model.width * AdjustedValues.ratio
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
                        item.settings.visibleReplyToUnfollowedUsers = model.visibleReplyToUnfollowedUsers

                        item.settings.updateSeenNotification = settingDialog.settings.updateSeenNotification
                        item.settings.sequentialDisplayOfPosts = (settingDialog.settings.displayOfPosts === "sequential")
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

    Rectangle {
        id: columnCursor
        radius: 3
        color: "transparent"
        opacity: 0
        border.width: 1
        border.color: Material.color(Material.LightBlue)
        layer.enabled: true
        layer.effect: Glow {
            radius: 8
            samples: 17
            color: Material.color(Material.LightBlue)
        }

        NumberAnimation {
            id: columnCursorAnimation
            target: columnCursor
            property: "opacity"
            duration: 500
            from: 1.0
            to: 0.0
            easing.type: Easing.Linear
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
            font.pointSize: AdjustedValues.f10
            text: hoveredLinkFrame.text
        }
    }

    ImageFullView {
        id: imageFullView
        anchors.fill: parent
        visible: false
    }
}
