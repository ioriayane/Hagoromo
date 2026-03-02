import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

Dialog {
    id: draftConfirmationDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    closePolicy: Dialog.CloseOnEscape

    bottomPadding: AdjustedValues.s5

    title: qsTr("Save draft?")

    RowLayout {
        Button {
            font.pointSize: AdjustedValues.f10
            flat: true
            text: qsTr("Keep editing")
            onClicked: draftConfirmationDialog.reject()
        }
        Button {
            font.pointSize: AdjustedValues.f10
            text: qsTr("Discard")
            onClicked: {
                draftConfirmationDialog.discarded()
                draftConfirmationDialog.close()
            }
        }
        Button {
            font.pointSize: AdjustedValues.f10
            font.bold: true
            text: qsTr("Save draft")
            onClicked: draftConfirmationDialog.accept()
        }
    }
}
