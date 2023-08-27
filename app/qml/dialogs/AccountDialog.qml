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

            Layout.preferredWidth: 400 * AdjustedValues.ratio
            Layout.preferredHeight: 300 * AdjustedValues.ratio

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
                            MenuItem {
                                icon.source: "../images/account_icon.png"
                                text: qsTr("Set as main")
                                onTriggered: accountList.model.setMainAccount(model.index)
                            }
                            MenuItem {
                                icon.source: "../images/visibility_on.png"
                                text: qsTr("Content filter")
                                onTriggered: {
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
