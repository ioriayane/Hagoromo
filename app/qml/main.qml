import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.accountlistmodel 1.0
import tech.relog.hagoromo.columnlistmodel 1.0
import tech.relog.hagoromo.listslistmodel 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "controls"
import "dialogs"
import "view"
import "parts"
import "compat"

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

    property bool visibleDialogs: settingDialog.visible ||
                                  postDialog.visible ||
                                  searchDialog.visible ||
                                  addColumnDialog.visible ||
                                  accountDialog.visible ||
                                  discoverFeedsDialog.visible ||
                                  columnsettingDialog.visible ||
                                  reportDialog.visible ||
                                  reportAccountDialog.visible ||
                                  addToListDialog.visible ||
                                  addListDialog.visible ||
                                  editProfileDialog.visible ||
                                  selectThreadGateDialog.visible ||
                                  messageDialog.visible ||
                                  imageFullView.visible

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

    SettingsC {
        //        property alias x: appWindow.x
        //        property alias y: appWindow.y
        property alias width: appWindow.width
        property alias height: appWindow.height
    }
    QtObject {
        id: translatorChanger
        objectName: "translatorChanger"
        signal triggered(string lang)
    }

    SystemTool {
        id: systemTool
    }

    ApplicationShortcut {
        enabled: !appWindow.visibleDialogs
        postDialogShortcut.onActivated: postDialog.open()
        searchDialogShortcut.onActivated: searchDialog.open()
        onShowLeftColumn: scrollView.showLeft()
        onShowRightColumn: scrollView.showRight()
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
        onAccepted: {
            repeater.updateSettings(2)
            translatorChanger.triggered(settingDialog.settings.language)
        }
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
                                     component_type, false, 300000, 350,
                                     settingDialog.settings.imageLayoutType, column_name, searchDialog.searchText, [])
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
                                     selectedType, false, 300000, 400,
                                     settingDialog.settings.imageLayoutType, selectedName, selectedUri, [])
            scrollView.showRightMost()
        }
        onOpenSatisticsAndLogs: (account_uuid) => {
                                    console.log("onOpenSatisticsAndLogs:" + account_uuid)
                                    if(logViewDialog.account.set(accountListModel, account_uuid)){
                                        logViewDialog.open()
                                    }
                                }
        onOpenDiscoverFeeds: (account_uuid) => {
                                 console.log("onOpenDiscoverFeeds:" + account_uuid)
                                 if(discoverFeedsDialog.account.set(accountListModel, account_uuid)){
                                     discoverFeedsDialog.open()
                                     addColumnDialog.reject()
                                 }
                             }
        onErrorOccured: (account_uuid, code, message) => appWindow.errorHandler(account_uuid, code, message)
    }

    AccountDialog {
        id: accountDialog
        accountModel: accountListModel
        onClosed: accountListModel.syncColumn()
        onErrorOccured: (account_uuid, code, message) => appWindow.errorHandler(account_uuid, code, message)
        onRequestAddMutedWords: (account_uuid) => {
                                    if(addMutedWordDialog.account.set(accountListModel, account_uuid)){
                                        addMutedWordDialog.open()
                                    }
                                }
        onRequestStatisticsAndLogs: (account_uuid) => {
                                        if(logViewDialog.account.set(accountListModel, account_uuid)){
                                            logViewDialog.open()
                                        }
                                    }
    }

    DiscoverFeedsDialog {
        id: discoverFeedsDialog
        onAccepted: {
            columnManageModel.append(discoverFeedsDialog.account.uuid,
                                     4, false, 300000, 400,
                                     settingDialog.settings.imageLayoutType, selectedName, selectedUri, [])
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
                // 言語を切り替えたときに選択項目を変更しないと初期表示の言語が変わらないのでいったん変更
                autoLoadingIntervalCombo.currentIndex = -1
                imageLayoutCombobox.currentIndex = -1

                autoLoadingCheckbox.checked = columnManageModel.item(i, ColumnListModel.AutoLoadingRole)
                autoLoadingIntervalCombo.setByValue(columnManageModel.item(i, ColumnListModel.LoadingIntervalRole))
                columnWidthSlider.value = columnManageModel.item(i, ColumnListModel.WidthRole)
                componentType = columnManageModel.item(i, ColumnListModel.ComponentTypeRole)
                imageLayoutCombobox.setByValue(columnManageModel.item(i, ColumnListModel.ImageLayoutTypeRole))

                visibleLikeCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleLikeRole)
                visibleRepostCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleRepostRole)
                visibleFollowCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleFollowRole)
                visibleMentionCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleMentionRole)
                visibleReplyCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleReplyRole)
                visibleQuoteCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleQuoteRole)
                visibleReplyToUnfollowedUsersCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleReplyToUnfollowedUsersRole)
                visibleRepostOfOwnCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleRepostOfOwnRole)
                visibleRepostOfFollowingUsersCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleRepostOfFollowingUsersRole)
                visibleRepostOfUnfollowingUsersCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleRepostOfUnfollowingUsersRole)
                visibleRepostOfMineCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleRepostOfMineRole)
                visibleRepostByMeCheckBox.checked = columnManageModel.item(i, ColumnListModel.VisibleRepostByMeRole)

                aggregateReactionsCheckBox.checked = columnManageModel.item(i, ColumnListModel.AggregateReactionsRole)

                open()
            }
        }

        onAccepted: {
            var i = columnManageModel.indexOf(columnKey)
            if(i >= 0){
                columnManageModel.update(i, ColumnListModel.AutoLoadingRole, autoLoadingCheckbox.checked)
                columnManageModel.update(i, ColumnListModel.LoadingIntervalRole, autoLoadingIntervalCombo.currentValue)
                columnManageModel.update(i, ColumnListModel.WidthRole, columnWidthSlider.value)
                columnManageModel.update(i, ColumnListModel.ImageLayoutTypeRole, imageLayoutCombobox.currentValue)

                columnManageModel.update(i, ColumnListModel.VisibleLikeRole, visibleLikeCheckBox.checked)
                columnManageModel.update(i, ColumnListModel.VisibleRepostRole, visibleRepostCheckBox.checked)
                columnManageModel.update(i, ColumnListModel.VisibleFollowRole, visibleFollowCheckBox.checked)
                columnManageModel.update(i, ColumnListModel.VisibleMentionRole, visibleMentionCheckBox.checked)
                columnManageModel.update(i, ColumnListModel.VisibleReplyRole, visibleReplyCheckBox.checked)
                columnManageModel.update(i, ColumnListModel.VisibleQuoteRole, visibleQuoteCheckBox.checked)
                columnManageModel.update(i, ColumnListModel.VisibleReplyToUnfollowedUsersRole, visibleReplyToUnfollowedUsersCheckBox.checked)
                columnManageModel.update(i, ColumnListModel.VisibleRepostOfOwnRole, visibleRepostOfOwnCheckBox.checked)
                columnManageModel.update(i, ColumnListModel.VisibleRepostOfFollowingUsersRole, visibleRepostOfFollowingUsersCheckBox.checked)
                columnManageModel.update(i, ColumnListModel.VisibleRepostOfUnfollowingUsersRole, visibleRepostOfUnfollowingUsersCheckBox.checked)
                columnManageModel.update(i, ColumnListModel.VisibleRepostOfMineRole, visibleRepostOfMineCheckBox.checked)
                columnManageModel.update(i, ColumnListModel.VisibleRepostByMeRole, visibleRepostByMeCheckBox.checked)

                columnManageModel.update(i, ColumnListModel.AggregateReactionsRole, aggregateReactionsCheckBox.checked)

                repeater.updateSettings(0)
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
    ReportMessageDialog {
        id: reportMessageDialog
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
    AddMutedWordDialog {
        id: addMutedWordDialog
    }
    SelectThreadGateDialog {
        id: selectThreadGateDialog
        property string postUri: ""
        property string threadgateUri: ""
        property var callback
        onOpened: {
            selectThreadGateDialog.ready = false
            postDialog.recordOperator.setAccount(selectThreadGateDialog.account.service,
                                                 selectThreadGateDialog.account.did,
                                                 selectThreadGateDialog.account.handle,
                                                 selectThreadGateDialog.account.email,
                                                 selectThreadGateDialog.account.accessJwt,
                                                 selectThreadGateDialog.account.refreshJwt)
            postDialog.recordOperator.clear()
            postDialog.recordOperator.requestPostGate(postUri)
        }
        onAccepted: {
            console.log("Update threadgate\n  uri=" + postUri + "\n  tg_uri=" + threadgateUri +
                        "\n  type=" + selectedType + "\n  options=" + selectedOptions)
            console.log("\n  initialType=" + initialType + "\n  initialOptions=" + initialOptions)
            if(initialOptions.length === selectedOptions.length){
                var no_change_threadgate=true
                for(var i=0; i<initialOptions.length; i++){
                    if(initialOptions[i] !== selectedOptions[i]){
                        no_change_threadgate=false
                        break
                    }
                }
                if(no_change_threadgate && initialType === selectedType){
                    console.log("No change threadgate.")
                    return
                }
            }
            postDialog.recordOperator.setAccount(selectThreadGateDialog.account.service,
                                                 selectThreadGateDialog.account.did,
                                                 selectThreadGateDialog.account.handle,
                                                 selectThreadGateDialog.account.email,
                                                 selectThreadGateDialog.account.accessJwt,
                                                 selectThreadGateDialog.account.refreshJwt)
            postDialog.recordOperator.clear()
            postDialog.recordOperator.updateThreadGate(postUri,
                                                       threadgateUri,
                                                       selectedType,
                                                       selectedOptions)
            globalProgressFrame.text = qsTr("Updating 'Edit interaction settings' ...")
        }
        Connections {
            target: postDialog.recordOperator
            function onFinished(success, uri, cid) {
                if(success && selectThreadGateDialog.postUri.length > 0){
                    globalProgressFrame.text = ""
                    selectThreadGateDialog.postUri = ""
                    selectThreadGateDialog.threadgateUri = ""
                    if(selectThreadGateDialog.callback){
                        selectThreadGateDialog.callback(uri, selectThreadGateDialog.selectedType, selectThreadGateDialog.selectedOptions)
                    }
                    selectThreadGateDialog.clear()
                }
            }
            function onFinishedRequestPostGate(success, quote_enabled, uris){
                if(success){
                    selectThreadGateDialog.initialQuoteEnabled = quote_enabled
                }
                selectThreadGateDialog.ready = true
            }
        }
    }
    LogViewDialog {
        id: logViewDialog
        parentHeight: parent.height
        onErrorOccured: (uuid, code, message) => appWindow.errorHandler(uuid, code, message)

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
                            postDialog.quoteCid = cid
                            postDialog.quoteUri = uri
                            postDialog.quoteAvatar = avatar
                            postDialog.quoteDisplayName = display_name
                            postDialog.quoteHandle = handle
                            postDialog.quoteIndexedAt = indexed_at
                            postDialog.quoteText = text
                            postDialog.open()
                        }
        // ダイアログより前に出せない
        // onRequestViewImages: (index, paths, alts) => imageFullView.open(index, paths, alts)
        onRequestAddMutedWord: (account_uuid, text) => {
                                   console.log(account_uuid + ", " + text)
                                   if(addMutedWordDialog.account.set(accountListModel, account_uuid)){
                                       addMutedWordDialog.initialValue = text
                                       addMutedWordDialog.open()
                                   }
                               }
        onRequestUpdateThreadGate: (account_uuid, uri, threadgate_uri, type, rules, callback) => {
                                       if(selectThreadGateDialog.account.set(accountListModel, account_uuid)){
                                           selectThreadGateDialog.postUri = uri
                                           selectThreadGateDialog.threadgateUri = threadgate_uri
                                           selectThreadGateDialog.initialQuoteEnabled = true
                                           selectThreadGateDialog.initialType = type
                                           selectThreadGateDialog.initialOptions = rules
                                           selectThreadGateDialog.callback = callback
                                           selectThreadGateDialog.open()
                                       }
                                   }
        onHoveredLinkChanged: hoveredLinkFrame.text = hoveredLink
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
                                  listsListModel.load()
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
    ListsListModel {
        // リストのキャッシュ用
        id: listsListModel
        visibilityType: ListsListModel.VisibilityTypeCuration

        property int currentAccountIndex: -1
        function load(next){
            if(!next){
                currentAccountIndex = accountListModel.count - 1
            }
            if(currentAccountIndex < 0){
                globalProgressFrame.text = ""
                return
            }
            var row = accountListModel.count - currentAccountIndex - 1
            if(row < 0){
                globalProgressFrame.text = ""
                return
            }
            var handle = accountListModel.item(currentAccountIndex, AccountListModel.HandleRole)
            var accessJwt = accountListModel.item(currentAccountIndex, AccountListModel.AccessJwtRole)
            if(accessJwt.length === 0){
                console.log("Empty accessJwt. load next.")
                currentAccountIndex -= 1
                load(true)
            }else{
                setAccount(accountListModel.item(currentAccountIndex, AccountListModel.ServiceRole),
                           accountListModel.item(currentAccountIndex, AccountListModel.DidRole),
                           handle,
                           "email",
                           accessJwt,
                           accountListModel.item(currentAccountIndex, AccountListModel.RefreshJwtRole)
                           )
                actor = did
                searchTarget = "#cache"
                if(listsListModel.getLatest()){
                    globalProgressFrame.text = qsTr("Loading lists") +
                            " (" + handle + ") ... " + (row+1) + "/" + accountListModel.count
                }else{
                    globalProgressFrame.text = ""
                }
            }
        }

        onRunningChanged: {
            console.log("listsListModel:" + running)
            if(running){
                return
            }
            currentAccountIndex -= 1
            load(true)
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
                                postDialog.quoteCid = cid
                                postDialog.quoteUri = uri
                                postDialog.quoteAvatar = avatar
                                postDialog.quoteDisplayName = display_name
                                postDialog.quoteHandle = handle
                                postDialog.quoteIndexedAt = indexed_at
                                postDialog.quoteText = text
                                postDialog.open()
                            }
            onRequestMention: (account_uuid, handle) => {
                                  postDialog.postType = "normal"
                                  postDialog.defaultAccountUuid = account_uuid
                                  postDialog.postText.text = handle + " "
                                  postDialog.open()
                              }
            onRequestMessage: (account_uuid, did, current_column_key) => {
                                  var pos = columnManageModel.insertNext(current_column_key, account_uuid, 8, false, 300000, 350,
                                                                         settingDialog.settings.imageLayoutType,
                                                                         qsTr("Chat"), "", [did])
                                  repeater.updateSettings(1)
                                  scrollView.showColumn(pos)
                              }

            onRequestViewAuthorFeed: (account_uuid, did, handle) => {
                                         columnManageModel.append(account_uuid, 5, false, 300000, 350,
                                                                  settingDialog.settings.imageLayoutType, handle, did, [])
                                         scrollView.showRightMost()
                                     }
            onRequestViewImages: (index, paths, alts) => imageFullView.open(index, paths, alts)
            onRequestViewFeedGenerator: (account_uuid, name, uri) => {
                                            columnManageModel.append(account_uuid, 4, false, 300000, 400,
                                                                     settingDialog.settings.imageLayoutType, name, uri, [])
                                            scrollView.showRightMost()
                                        }
            onRequestViewSearchPosts: (account_uuid, text, current_column_key) => {
                                          console.log("Search:" + account_uuid + ", " + text + ", " + current_column_key)
                                          var pos = columnManageModel.insertNext(current_column_key, account_uuid, 2, false, 300000, 350,
                                                                                 settingDialog.settings.imageLayoutType,
                                                                                 qsTr("Search posts"), text, [])
                                          repeater.updateSettings(1)
                                          scrollView.showColumn(pos)
                                      }
            onRequestViewListFeed: (account_uuid, uri, name) => {
                                       console.log("uuid=" + account_uuid + "\nuri=" + uri + "\nname=" + name)
                                       columnManageModel.append(account_uuid, 6, false, 300000, 400,
                                                                settingDialog.settings.imageLayoutType, name, uri, [])
                                       scrollView.showRightMost()
                                   }

            onRequestReportPost: (account_uuid, uri, cid) => {
                                     if(reportDialog.account.set(accountListModel, account_uuid)){
                                         reportDialog.targetUri = uri
                                         reportDialog.targetCid = cid
                                         reportDialog.open()
                                     }
                                 }
            onRequestReportAccount: (account_uuid, did) => {
                                        if(reportAccountDialog.account.set(accountListModel, account_uuid)){
                                            reportAccountDialog.targetDid = did
                                            reportAccountDialog.open()
                                        }
                                    }
            onRequestReportMessage: (account_uuid, did, convo_id, message_id) => {
                                        if(reportMessageDialog.account.set(accountListModel, account_uuid)){
                                            console.log("onRequestReportMessage: account_uuid=" + account_uuid + ", did=" + did + ", convo_id=" + convo_id + ", message_id=" + message_id)
                                            reportMessageDialog.targetAccountDid = did
                                            reportMessageDialog.targetConvoId = convo_id
                                            reportMessageDialog.targetMessageId = message_id
                                            reportMessageDialog.open()
                                        }
                                    }

            onRequestAddRemoveFromLists: (account_uuid, did) => {
                                             if(addToListDialog.account.set(accountListModel, account_uuid)){
                                                 addToListDialog.targetDid = did
                                                 addToListDialog.open()
                                             }
                                         }
            onRequestAddMutedWord: (account_uuid, text) => {
                                       console.log(account_uuid + ", " + text)
                                       if(addMutedWordDialog.account.set(accountListModel, account_uuid)){
                                           addMutedWordDialog.initialValue = text
                                           addMutedWordDialog.open()
                                       }
                                   }

            onRequestEditProfile: (account_uuid, did, avatar, banner, display_name, description) => {
                                      if(editProfileDialog.account.set(accountListModel, account_uuid)){
                                          editProfileDialog.avatar = avatar
                                          editProfileDialog.banner = banner
                                          editProfileDialog.displayName = display_name
                                          editProfileDialog.description = description
                                          editProfileDialog.open()
                                      }
                                  }
            onRequestEditList: (account_uuid, uri, avatar, name, description) => {
                                   if(addListDialog.account.set(accountListModel, account_uuid)){
                                       addListDialog.editMode = true
                                       addListDialog.listUri = uri
                                       addListDialog.avatar = avatar
                                       addListDialog.displayName = name
                                       addListDialog.description = description
                                       addListDialog.open()
                                   }
                               }
            onRequestUpdateThreadGate: (account_uuid, uri, threadgate_uri, type, rules, callback) => {
                                           if(selectThreadGateDialog.account.set(accountListModel, account_uuid)){
                                               selectThreadGateDialog.postUri = uri
                                               selectThreadGateDialog.threadgateUri = threadgate_uri
                                               selectThreadGateDialog.initialType = type
                                               selectThreadGateDialog.initialOptions = rules
                                               selectThreadGateDialog.callback = callback
                                               selectThreadGateDialog.open()
                                           }
                                       }

            onRequestMoveToLeft: (key) => {
                                     console.log("move to left:" + key)
                                     columnManageModel.move(key, ColumnListModel.MoveLeft)
                                     repeater.updateSettings(1)
                                 }
            onRequestMoveToRight: (key) => {
                                      console.log("move to right:" + key)
                                      columnManageModel.move(key, ColumnListModel.MoveRight)
                                      repeater.updateSettings(1)
                                  }
            onRequestRemove: (key) => {
                                 console.log("remove column:" + key)
                                 columnManageModel.removeByKey(key)
                                 repeater.updateSettings(1)
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

            property int prevShowPosition: 0
            property int rows: settingDialog.settings.rowCount
            property int childHeight: scrollView.height - scrollView.ScrollBar.horizontal.height
            contentHeight: childHeight

            function showLeftMost() {
                showColumn(0)
            }

            function showRightMost() {
                // scrollView.contentItemはFlickable
                // Flickableの幅が全カラムの幅より小さくなったら移動させる
                showColumn(repeater.count - 1)
            }

            function showLeft() {
                showColumn(columnManageModel.moveSelectionToLeft())
            }

            function showRight() {
                showColumn(columnManageModel.moveSelectionToRight())
            }

            function showColumn(position){
                var row_list = columnManageModel.getRowListInOrderOfPosition()
                var item = undefined
                if(row_list.length <= position){
                    return
                }
                var direction = "left"
                if(prevShowPosition < position){
                    direction = "right"
                }
                prevShowPosition = position

                item = repeater.itemAt(row_list[position])   //ここのitemはloader自身
                if(item){
                    if((item.x + item.width) > scrollView.contentItem.width && direction === "right"){
                        scrollView.contentItem.contentX = (item.x + item.width) - scrollView.contentItem.width
                    }else if(item.x < scrollView.contentItem.contentX){
                        scrollView.contentItem.contentX = item.x
                    }
                    columnCursor.width = item.width
                    columnCursor.height = item.height
                    columnCursor.x = item.x
                    columnCursor.y = item.y
                    columnCursorAnimation.stop()
                    columnCursorAnimation.start()
                    // 選択位置の更新
                    columnManageModel.moveSelection(position)
                    repeater.updateSelection()
                }
            }

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
                    updateContentWidth()
                }

                function updateSelection() {
                    for(var i=0; i<repeater.count; i++){
                        var item = repeater.itemAt(i)   //ここのitemはloader自身
                        item.updateSelection()
                    }
                }

                // target
                //  0:setting only
                //  1:layout only
                //  2:both
                function updateSettings(target) {
                    // モデルの順番を入れ替えるとLoader側が対応できないのと
                    // 最左にくるものから処理しないとレイアウトが循環して矛盾するため
                    // カラムの管理順ではなくポジションの順番で処理する
                    var row_list = columnManageModel.getRowListInOrderOfPosition()
                    for(var i=0; i<row_list.length; i++){
                        var item = repeater.itemAt(row_list[i])   //ここのitemはloader自身
                        if(target === 0){
                            item.setSettings()
                        }else if(target === 1){
                            item.setLayout()
                            item.updateSelection()
                        }else{
                            item.setSettings()
                            item.setLayout()
                            item.updateSelection()
                        }
                    }
                    updateContentWidth()
                }

                function updateContentWidth() {
                    var w = []
                    var prev_row = -1
                    var row_list = columnManageModel.getRowListInOrderOfPosition()
                    for(var i=0; i<row_list.length; i++){
                        var item = repeater.itemAt(row_list[i])   //ここのitemはloader自身
                        if(prev_row !== item.row){
                            w[item.row] = 0
                            prev_row = item.row
                        }
                        w[item.row] += item.width + item.anchors.leftMargin
                    }
                    var max_w = 0
                    for(i=0; i<w.length; i++){
                        max_w = (max_w < w[i]) ? w[i] : max_w
                    }
                    scrollView.contentWidth = max_w
                }

                function updateAccount(account_uuid){
                    for(var i=0; i<repeater.count; i++){
                        var item = repeater.itemAt(i)   //ここのitemはloader自身
                        if(item.item.account.uuid === account_uuid){
                            if(item.item.account.set(accountListModel, item.item.account.uuid)){
                                console.log("Update column : col=" + i + ", a_uuid=" + item.item.account.uuid)
                                item.item.reflect()
                            }
                        }
                    }
                }

                Loader {
                    id: loader
                    y: {
                        // row * height
                        if(scrollView.rows === 1){
                            return 0
                        }else if(scrollView.rows === 2){
                            return row * scrollView.childHeight * settingDialog.settings.rowHeightRatio2 / 100
                        }else{
                            if(row === 0){
                                return 0
                            }else if(row === 1){
                                return scrollView.childHeight * settingDialog.settings.rowHeightRatio31 / 100
                            }else{
                                return scrollView.childHeight * settingDialog.settings.rowHeightRatio32 / 100
                            }
                        }
                    }
                    height: {
                        // scrollView.childHeight / scrollView.rows
                        if(scrollView.rows === 1){
                            return scrollView.childHeight
                        }else if(scrollView.rows === 2){
                            if(row === 0){
                                return scrollView.childHeight * settingDialog.settings.rowHeightRatio2 / 100
                            }else{
                                return scrollView.childHeight * (100 - settingDialog.settings.rowHeightRatio2) / 100
                            }
                        }else{
                            if(row === 0){
                                return scrollView.childHeight * settingDialog.settings.rowHeightRatio31 / 100
                            }else if(row === 1){
                                return scrollView.childHeight * (settingDialog.settings.rowHeightRatio32 - settingDialog.settings.rowHeightRatio31) / 100
                            }else{
                                return scrollView.childHeight * (100 - settingDialog.settings.rowHeightRatio32) / 100
                            }
                        }
                    }
                    width: model.width * AdjustedValues.ratio
                    sourceComponent: columnView
                    property int row: 0

                    onLoaded: {
                        // Loaderで読み込んだComponentにアカウント情報など設定する
                        var row = accountListModel.indexAt(model.accountUuid)
                        console.log("(1) loader:" + row + ", " + model.accountUuid)
                        if(!item.account.set(accountListModel, model.accountUuid)){
                            return
                        }

                        item.columnKey = model.key
                        item.componentType = model.componentType
                        item.selected = model.selected
                        setSettings()
                        item.load()

                        if(model.index === columnManageModel.rowCount() - 1){
                            // 最後の時にレイアウトを設定する
                            // 読み込んでいる過程では左がいない場合がある
                            repeater.updateSettings(1)
                        }
                    }

                    function setLayout() {
                        var cur_pos = columnManageModel.getPosition(model.index)
                        var left_index = columnManageModel.getPreviousRow(model.index)
                        var count_in_row = Math.ceil(repeater.count / scrollView.rows)
                        var index_in_row = cur_pos % count_in_row
                        loader.row = Math.trunc(cur_pos / count_in_row)
                        console.log("setLayout(1)   :" + model.index + ": row=" + loader.row + ", index_in_row=" + index_in_row + ", count_in_row=" + count_in_row)
                        if(index_in_row === 0){
                            console.log("setLayout(2.1) :" + model.index + ": left_pos=" + left_index + ", left is " + loader.parent)
                            loader.anchors.left = loader.parent.left
                            loader.anchors.leftMargin = 0
                        }else{
                            console.log("setLayout(2.2) :" + model.index + ": left_pos=" + left_index + ", left name=" + repeater.itemAt(left_index).item.settings.columnName)
                            loader.anchors.left = repeater.itemAt(left_index).right
                            loader.anchors.leftMargin = 3
                        }
                    }

                    function setSettings() {
                        item.settings.autoLoading = model.autoLoading
                        item.settings.loadingInterval = model.loadingInterval
                        item.settings.columnName = model.name
                        item.settings.columnValue = model.value
                        item.settings.columnValueList = model.valueList
                        item.settings.imageLayoutType = model.imageLayoutType

                        item.settings.visibleLike = model.visibleLike
                        item.settings.visibleRepost = model.visibleRepost
                        item.settings.visibleFollow = model.visibleFollow
                        item.settings.visibleMention = model.visibleMention
                        item.settings.visibleReply = model.visibleReply
                        item.settings.visibleQuote = model.visibleQuote
                        item.settings.visibleReplyToUnfollowedUsers = model.visibleReplyToUnfollowedUsers
                        item.settings.visibleRepostOfOwn = model.visibleRepostOfOwn
                        item.settings.visibleRepostOfFollowingUsers = model.visibleRepostOfFollowingUsers
                        item.settings.visibleRepostOfUnfollowingUsers = model.visibleRepostOfUnfollowingUsers
                        item.settings.visibleRepostOfMine = model.visibleRepostOfMine
                        item.settings.visibleRepostByMe = model.visibleRepostByMe
                        item.settings.aggregateReactions = model.aggregateReactions

                        item.settings.updateSeenNotification = settingDialog.settings.updateSeenNotification
                        item.settings.sequentialDisplayOfPosts = (settingDialog.settings.displayOfPosts === "sequential")
                    }

                    function updateSelection() {
                        item.selected = model.selected
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
                layer.effect: GlowC {
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
        }
    }

    Frame {
        // ハイパーリンクの内容を表示する
        id: hoveredLinkFrame
        anchors.left: rootLayout.left
        anchors.bottom: rootLayout.bottom
        anchors.leftMargin: scrollView.x
        anchors.bottomMargin: scrollView.ScrollBar.horizontal.height + 5
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

    Frame {
        // 何かの読み込み中の表示
        id: globalProgressFrame
        anchors.right: rootLayout.right
        anchors.bottom: rootLayout.bottom
        anchors.rightMargin: 5
        anchors.bottomMargin: scrollView.ScrollBar.horizontal.height + 5
        visible: globalProgressFrame.text.length > 0
        background: Rectangle {
            radius: 3
            border.width: 1
            border.color: Material.frameColor
            color: Material.backgroundColor
        }
        property string text: ""
        RowLayout {
            BusyIndicator {
                Layout.preferredWidth: AdjustedValues.i24
                Layout.preferredHeight: AdjustedValues.i24
                running: globalProgressFrame.visible
            }
            Label {
                font.pointSize: AdjustedValues.f10
                text: globalProgressFrame.text
            }
        }
    }

    ImageFullView {
        id: imageFullView
        anchors.fill: parent
        visible: false
    }

    Component.onCompleted: {
        if(accountListModel.count > 0){
            globalProgressFrame.text = qsTr("Loading account(s) ...")
        }
        accountListModel.load()
    }
}
