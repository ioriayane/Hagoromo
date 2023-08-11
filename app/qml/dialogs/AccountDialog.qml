import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.accountlistmodel 1.0
import "../controls"
import "../parts"

Dialog {
    id: accountDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Account management")

    property alias accountModel: accountList.model

    LoginDialog {
        id: login
        onAccepted: {
            accountModel.updateAccount(session.service, session.identifier, session.password,
                                       session.did, session.handle, session.email,
                                       session.accessJwt, session.refreshJwt,
                                       session.authorized)
            accountModel.updateAccountProfile(session.service, session.identifier)
        }
    }
    ContentFilterSettingDialog {
        id: contentFilter
    }

    ColumnLayout {

        ListView {
            id: accountList

            Layout.preferredWidth: 400
            Layout.preferredHeight: 300

            footer: ItemDelegate {
                width: accountList.width
                padding: 3
                Image {
                    anchors.centerIn: parent
                    width: 24
                    height: 24
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
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        source: model.avatar
                    }
                    Label {
                        text: model.handle
                        elide: Text.ElideRight
                    }
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                    }
//                    IconButton {
//                        // プロフィールの編集
//                        Layout.preferredWidth: 36
//                        Layout.preferredHeight: 26
//                        display: AbstractButton.IconOnly
//                        iconSource: "../images/edit.png"
//                        iconSize: 18
//                    }
                    IconButton {
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 26
                        display: AbstractButton.IconOnly
                        iconSource: "../images/visibility_on.png"
                        iconSize: 18
                        onClicked: {
                            var i = model.index
                            contentFilter.account.service = accountList.model.item(i, AccountListModel.ServiceRole)
                            contentFilter.account.did = accountList.model.item(i, AccountListModel.DidRole)
                            contentFilter.account.handle = accountList.model.item(i, AccountListModel.HandleRole)
                            contentFilter.account.email = accountList.model.item(i, AccountListModel.EmailRole)
                            contentFilter.account.accessJwt = accountList.model.item(i, AccountListModel.AccessJwtRole)
                            contentFilter.account.refreshJwt = accountList.model.item(i, AccountListModel.RefreshJwtRole)
                            contentFilter.account.avatar = accountList.model.item(i, AccountListModel.AvatarRole)
                            contentFilter.open()
                        }
                    }
                    IconButton {
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 26
                        display: AbstractButton.IconOnly
                        iconSource: "../images/delete.png"
                        iconSize: 18
                        onClicked: accountList.model.removeAccount(model.index)
                    }
                }
            }

        }
        Button {
            text: qsTr("Close")
            flat: true
            onClicked: accountDialog.close()
        }
    }
}
