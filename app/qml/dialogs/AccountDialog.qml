import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.accountlistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../parts"

Dialog {
    id: accountDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Account management")

    property alias accountModel: accountList.model
    signal errorOccured(string account_uuid, string code, string message)

    signal requestAddMutedWords(int account_index)

    LoginDialog {
        id: login
        onAccepted: {
            accountModel.updateAccount(session.service, session.identifier, session.password,
                                       session.did, session.handle, session.email,
                                       session.accessJwt, session.refreshJwt,
                                       session.authorized)
            accountModel.updateAccountProfile(session.service, session.identifier)
        }
        onErrorOccured: (code, message) => accountDialog.errorOccured("", code, message)
    }
    ContentFilterSettingDialog {
        id: contentFilter
    }
    BlockedAccountsDialog {
        id: blockedAccountsDialog
    }
    MutedAccountsDialog {
        id: mutedAccountsDialog
    }
    BlockedListsDialog {
        id: blockedListsDialog
    }
    MutedListsDialog {
        id: mutedListsDialog
    }
    LogViewDialog {
        id: logViewDialog
        parentHeight: accountDialog.parent.height
        onErrorOccured: (uuid, code, message) => accountDialog.errorOccured(uuid, code, message)
    }

    SelectThreadGateDialog {
        id: selectThreadGateDialog
        defaultSettingMode: true
        property int accountIndex: -1
        onAccepted: {
            accountList.model.update(accountIndex, AccountListModel.ThreadGateTypeRole, selectedType)
            accountList.model.update(accountIndex, AccountListModel.ThreadGateOptionsRole, selectedOptions)
        }
    }

    ColumnLayout {

        ListView {
            id: accountList
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            Layout.preferredHeight: 300 * AdjustedValues.ratio
            maximumFlickVelocity: AdjustedValues.maximumFlickVelocity

            footer: ItemDelegate {
                width: accountList.width
                height: implicitHeight * AdjustedValues.ratio
                padding: 3
                Image {
                    anchors.centerIn: parent
                    width: AdjustedValues.i24
                    height: AdjustedValues.i24
                    source: "../images/add_user.png"
                    layer.enabled: true
                    layer.effect: ColorOverlay {
                        color: Material.accentColor // Material.color(Material.Grey)
                    }
                }
                onClicked: {
                    login.serviceText = "https://bsky.social"
                    login.idText = ""
                    login.passwordText = ""
                    login.open()
                }
            }

            delegate: ItemDelegate {
                width: accountList.width
                height: implicitHeight * AdjustedValues.ratio
                onClicked: {
                    var i = model.index
                    login.serviceText = accountList.model.item(i, AccountListModel.ServiceRole)
                    login.idText = accountList.model.item(i, AccountListModel.IdentifierRole)
                    login.passwordText = accountList.model.item(i, AccountListModel.PasswordRole)
                    login.open()
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    spacing: 5
                    AvatarImage {
                        Layout.preferredWidth: AdjustedValues.i24
                        Layout.preferredHeight: AdjustedValues.i24
                        source: model.avatar
                    }
                    Label {
                        font.pointSize: AdjustedValues.f10
                        text: model.handle
                        elide: Text.ElideRight
                    }
                    TagLabel {
                        source: ""
                        text: " Main "
                        color: Material.primaryColor
                        fontPointSize: AdjustedValues.f8
                        visible: model.isMain
                    }
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                    }
                    IconButton {
                        Layout.preferredHeight: AdjustedValues.b26
                        iconSource: "../images/more.png"
                        onClicked: moreMenu.open()
                        Menu {
                            id: moreMenu
                            width: {
                                var w = mutedWordMenuItem.implicitWidth
                                if(threadGateMenuItem.implicitWidth > w){
                                    w = threadGateMenuItem.implicitWidth
                                }
                                if(statsAndLogsMenutItem.implicitWidth > w){
                                    w = statsAndLogsMenutItem.implicitWidth
                                }
                                return w
                            }
                            MenuItem {
                                icon.source: "../images/account_icon.png"
                                text: qsTr("Set as main")
                                onTriggered: accountList.model.setMainAccount(model.index)
                            }
                            MenuItem {
                                id: statsAndLogsMenutItem
                                icon.source: "../images/database.png"
                                text: qsTr("Post statistics and logs")
                                onTriggered: {
                                    if(logViewDialog.account.set(accountList.model, model.uuid)){
                                        logViewDialog.open()
                                    }
                                }
                            }
                            MenuSeparator {}
                            MenuItem {
                                icon.source: "../images/visibility_on.png"
                                text: qsTr("Content filter")
                                onTriggered: {
                                    if(contentFilter.account.set(accountList.model, model.uuid)){
                                        contentFilter.open()
                                    }
                                }
                            }
                            Menu {
                                title: qsTr("Mute")
                                width: mutedAccountsMenuItem.implicitWidth
                                MenuItem {
                                    id: mutedWordMenuItem
                                    icon.source: "../images/mute.png"
                                    text: qsTr("Muted words and tags")
                                    onTriggered: accountDialog.requestAddMutedWords(model.index)
                                }
                                MenuItem {
                                    id: mutedAccountsMenuItem
                                    icon.source: "../images/account_off.png"
                                    text: qsTr("Muted accounts")
                                    onTriggered: {
                                        if(mutedAccountsDialog.account.set(accountList.model, model.uuid)){
                                            mutedAccountsDialog.open()
                                        }
                                    }
                                }
                                MenuItem {
                                    id: mutedlistsMenuItem
                                    icon.source: "../images/account_off.png"
                                    text: qsTr("Muted lists")
                                    onTriggered: {
                                        if(mutedListsDialog.account.set(accountList.model, model.uuid)){
                                            mutedListsDialog.open()
                                        }
                                    }
                                }
                            }
                            Menu {
                                title: qsTr("Block")
                                width: blockedAccountsMenuItem.implicitWidth
                                MenuItem {
                                    id: blockedAccountsMenuItem
                                    icon.source: "../images/block.png"
                                    text: qsTr("Blocked accounts")
                                    onTriggered: {
                                        if(blockedAccountsDialog.account.set(accountList.model, model.uuid)){
                                            blockedAccountsDialog.open()
                                        }
                                    }
                                }
                                MenuItem {
                                    id: blockedlistsMenuItem
                                    icon.source: "../images/block.png"
                                    text: qsTr("Blocked lists")
                                    onTriggered: {
                                        if(blockedListsDialog.account.set(accountList.model, model.uuid)){
                                            blockedListsDialog.open()
                                        }
                                    }
                                }
                            }
                            MenuSeparator {}
                            MenuItem {
                                id: threadGateMenuItem
                                icon.source: "../images/thread.png"
                                text: qsTr("Who can reply")
                                onTriggered: {
                                    if(selectThreadGateDialog.account.set(accountList.model, model.uuid)){
                                        var i = model.index
                                        selectThreadGateDialog.initialType = accountList.model.item(i, AccountListModel.ThreadGateTypeRole)
                                        selectThreadGateDialog.initialOptions = accountList.model.item(i, AccountListModel.ThreadGateOptionsRole)
                                        selectThreadGateDialog.accountIndex = i
                                        selectThreadGateDialog.open()
                                    }
                                }
                            }
                            MenuSeparator {}
                            MenuItem {
                                icon.source: "../images/delete.png"
                                text: qsTr("Remove account")
                                onTriggered: accountList.model.removeAccount(model.index)
                            }
                        }
                    }
                }
            }
        }
        Button {
            font.pointSize: AdjustedValues.f10
            text: qsTr("Close")
            flat: true
            onClicked: accountDialog.close()
        }
    }
}
