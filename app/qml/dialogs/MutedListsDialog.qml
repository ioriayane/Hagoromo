import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.listmuteslistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"
import "../view"

Dialog {
    id: mutedListsDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    closePolicy: Popup.NoAutoClose
    title: qsTr("Muted lists")

    property alias account: account
    Account {
        id: account
    }
    signal errorOccured(string account_uuid, string code, string message)

    onOpened: {
        if(account.uuid.length === 0){
            return
        }
        listMutesListModel.setAccount(account.uuid)
        listMutesListModel.getLatest()

        listScrollView.currentIndex = -1
    }
    onClosed: {
        listMutesListModel.clear()
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
        ListsListView {
            id: listScrollView
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            Layout.preferredHeight: 350 * AdjustedValues.ratioHalf
            mode: 1
            model: ListMutesListModel {
                id: listMutesListModel
                onErrorOccured: (code, message) => mutedListsDialog.errorOccured(mutedListsDialog.account.uuid, code, message)
            }
        }
        RowLayout {
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Close")
                flat: true
                onClicked: mutedListsDialog.close()
            }
            Item {
                Layout.fillWidth: true
            }
        }
    }
}
