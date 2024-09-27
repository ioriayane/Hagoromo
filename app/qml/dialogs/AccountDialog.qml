import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.accountlistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../parts"
import "../compat"

Dialog {
    id: accountDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Account management")

    property alias accountModel: accountList.model
    signal errorOccured(string account_uuid, string code, string message)

    signal requestAddMutedWords(string account_uuid)
    signal requestStatisticsAndLogs(string account_uuid)

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

    SelectThreadGateDialog {
        id: selectThreadGateDialog
        defaultSettingMode: true
        property int accountIndex: -1
        onAccepted: {
            accountList.model.update(accountIndex, AccountListModel.PostGateQuoteEnabledRole, selectedQuoteEnabled)
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
                    layer.effect: ColorOverlayC {
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
                    TagLabel {
                        source: ""
                        text: " Please login"
                        color: Material.color(Material.Red)
                        fontPointSize: AdjustedValues.f8
                        visible: !model.authorized
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                    }
                    IconButton {
                        Layout.preferredHeight: AdjustedValues.b26
                        iconSource: "../images/more.png"
                        onClicked: moreMenu.open()
                        MenuEx {
                            id: moreMenu
                            Action {
                                icon.source: "../images/account_icon.png"
                                text: qsTr("Set as main")
                                onTriggered: accountList.model.setMainAccount(model.index)
                            }
                            Action {
                                icon.source: "../images/database.png"
                                text: qsTr("Statistics and logs")
                                onTriggered: accountDialog.requestStatisticsAndLogs(model.uuid)
                            }
                            MenuSeparator {}
                            Action {
                                icon.source: "../images/visibility_on.png"
                                text: qsTr("Content filter")
                                onTriggered: {
                                    if(contentFilter.account.set(accountList.model, model.uuid)){
                                        contentFilter.open()
                                    }
                                }
                            }
                            MenuEx {
                                title: qsTr("Mute")
                                Action {
                                    icon.source: "../images/mute.png"
                                    text: qsTr("Muted words and tags")
                                    onTriggered: accountDialog.requestAddMutedWords(model.uuid)
                                }
                                Action {
                                    icon.source: "../images/account_off.png"
                                    text: qsTr("Muted accounts")
                                    onTriggered: {
                                        if(mutedAccountsDialog.account.set(accountList.model, model.uuid)){
                                            mutedAccountsDialog.open()
                                        }
                                    }
                                }
                                Action {
                                    icon.source: "../images/account_off.png"
                                    text: qsTr("Muted lists")
                                    onTriggered: {
                                        if(mutedListsDialog.account.set(accountList.model, model.uuid)){
                                            mutedListsDialog.open()
                                        }
                                    }
                                }
                            }
                            MenuEx {
                                title: qsTr("Block")
                                Action {
                                    icon.source: "../images/block.png"
                                    text: qsTr("Blocked accounts")
                                    onTriggered: {
                                        if(blockedAccountsDialog.account.set(accountList.model, model.uuid)){
                                            blockedAccountsDialog.open()
                                        }
                                    }
                                }
                                Action {
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
                            Action {
                                icon.source: "../images/thread.png"
                                text: qsTr("Post interaction settings")
                                onTriggered: {
                                    if(selectThreadGateDialog.account.set(accountList.model, model.uuid)){
                                        var i = model.index
                                        selectThreadGateDialog.initialQuoteEnabled = accountList.model.item(i, AccountListModel.PostGateQuoteEnabledRole)
                                        selectThreadGateDialog.initialType = accountList.model.item(i, AccountListModel.ThreadGateTypeRole)
                                        selectThreadGateDialog.initialOptions = accountList.model.item(i, AccountListModel.ThreadGateOptionsRole)
                                        selectThreadGateDialog.accountIndex = i
                                        selectThreadGateDialog.open()
                                    }
                                }
                            }
                            MenuSeparator {}
                            Action {
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
