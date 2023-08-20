import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1 as P

import tech.relog.hagoromo.recordoperator 1.0
import tech.relog.hagoromo.accountlistmodel 1.0

import "../controls"
import "../parts"

Dialog {
    id: searchDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    closePolicy: Popup.NoAutoClose

    property int parentWidth: parent.width
    property alias accountModel: accountCombo.model
    property alias selectedAccountIndex: accountCombo.currentIndex

    property string defaultAccountUuid: ""
    property string searchType: "posts"
    property string searchText: searchText.text

    onOpened: {
        var i = accountModel.indexAt(defaultAccountUuid)
        accountCombo.currentIndex = -1
        if(i >= 0){
            accountCombo.currentIndex = i
        } else {
            accountCombo.currentIndex = 0
        }
        searchText.forceActiveFocus()
    }
    onClosed: {
        defaultAccountUuid = ""
        searchType = "posts"
        searchText.clear()
    }

    Shortcut {  // Post
        enabled: postButton.enabled && searchText.focus
        sequence: "Ctrl+Return"
        onActivated: postButton.clicked()
    }
    Shortcut {  // Close
        // DialogのclosePolicyでEscで閉じられるけど、そのうち編集中の確認ダイアログを
        // 入れたいので別でイベント処理をする。onClosedで閉じるをキャンセルできなさそうなので。
        enabled: searchDialog.visible && ! postButton.enabled
        sequence: "Esc"
        onActivated: searchDialog.close()
    }

    ButtonGroup {
        id: searchTypeButtonGroup
        buttons: searchTypeRowlayout.children
    }

    ColumnLayout {

        RowLayout {
            AvatarImage {
                id: accountAvatarImage
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                //                source:
            }

            ComboBox {
                id: accountCombo
                Layout.preferredWidth: 200
                textRole: "handle"
                valueRole: "did"
                delegate: ItemDelegate {
                    text: model.handle
                    width: parent.width
                    onClicked: accountCombo.currentIndex = model.index
                }
                onCurrentIndexChanged: {
                    if(accountCombo.currentIndex >= 0){
                        accountAvatarImage.source =
                                searchDialog.accountModel.item(accountCombo.currentIndex, AccountListModel.AvatarRole)
                    }
                }
            }
        }

        RowLayout {
            id: searchTypeRowlayout
            RadioButton {
                property string value: "posts"
                text: qsTr("Posts")
                checked: true
            }
            RadioButton {
                property string value: "users"
                text: qsTr("Users")
            }
        }

        TextField  {
            id: searchText
            Layout.preferredWidth: 300
            selectByMouse: true
        }

        RowLayout {
            //            Layout.alignment: Qt.AlignRight
            Button {
                flat: true
                text: qsTr("Cancel")
                onClicked: searchDialog.close()
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            Button {
                id: postButton
                Layout.alignment: Qt.AlignRight
                enabled: searchText.text.length > 0
                text: qsTr("Search")
                onClicked: {
                    searchDialog.searchType = searchTypeButtonGroup.checkedButton.value
                    searchDialog.accept()
                }
            }
        }
    }
}
