import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

ColumnLayout {
    spacing: 0
    property alias postText: postText
    property alias getPostTextButton: getPostTextButton
    property alias getPostTextButtonBusy: getPostTextButtonBusy

    Label {
        id: postText
        Layout.fillWidth: true
        visible: text.length > 0
        textFormat: Text.StyledText
        wrapMode: Text.WrapAnywhere
        font.pointSize: AdjustedValues.f10
        lineHeight: 1.3
    }
    Button {
        id: getPostTextButton
        Layout.alignment: Qt.AlignRight
        Layout.preferredHeight: AdjustedValues.b36
        visible: false
        enabled: !getPostTextButtonBusy.visible
        text: qsTr("Get unblurred text")
        BusyIndicator {
            id: getPostTextButtonBusy
            anchors.fill: parent
            visible: false
        }
    }
}
