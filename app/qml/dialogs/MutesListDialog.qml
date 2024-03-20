import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.muteslistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"
import "../view"

Dialog {
    id: mutesListDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    closePolicy: Popup.NoAutoClose
    title: qsTr("Muted accounts")

    property alias account: account
    Account {
        id: account
    }
    signal errorOccured(string account_uuid, string code, string message)

    onOpened: {
        if(account.service.length === 0){
            return
        }
        mutesListModel.setAccount(account.service, account.did, account.handle,
                                  account.email, account.accessJwt, account.refreshJwt)
        mutesListModel.getLatest()

        profileListScrollView.currentIndex = -1
    }
    onClosed: {
        mutesListModel.clear()
    }

    ColumnLayout {
        spacing: 10
        RowLayout {
            AvatarImage {
                Layout.preferredWidth: AdjustedValues.i24
                Layout.preferredHeight: AdjustedValues.i24
                source: account.avatar
            }
            Label {
                Layout.fillWidth: true
                font.pointSize: AdjustedValues.f10
                text: account.handle
                elide: Text.ElideRight
            }
        }
        ProfileListView {
            id: profileListScrollView
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            Layout.preferredHeight: 350 * AdjustedValues.ratioHalf
            viewMode: 2
            model: MutesListModel {
                id: mutesListModel
                onErrorOccured: (code, message) => mutesListDialog.errorOccured(mutesListDialog.account.uuid, code, message)
            }
        }
        RowLayout {
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Close")
                flat: true
                onClicked: mutesListDialog.close()
            }
            Item {
                Layout.fillWidth: true
            }
        }
    }
}
