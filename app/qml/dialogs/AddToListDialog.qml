import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.listslistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"

Dialog {
    id: addToListDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Add/Remove from lists")

    property string targetDid: ""
    property alias account: account
    Account {
        id: account
    }
    signal errorOccured(string account_uuid, string code, string message)

    onOpened: {
        if(account.service.length === 0){
            return
        }

    }
    onClosed: {
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
                font.pointSize: AdjustedValues.f10
                text: account.handle
                elide: Text.ElideRight
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
        }
        Label {
            text: "handle=" + account.handle + "\n target=" + targetDid
        }

        RowLayout {
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Close")
                flat: true
                onClicked: addToListDialog.reject()
            }
            Item {
                Layout.fillWidth: true
            }
        }
    }
}
