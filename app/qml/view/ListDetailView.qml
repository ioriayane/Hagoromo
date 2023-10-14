import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"

ColumnLayout {
    id: listDetailView
    Layout.fillWidth: true
    Layout.fillHeight: true
    spacing: 0

    property string hoveredLink: ""

    property alias model: relayObject
    property string listUri: ""

    signal errorOccured(string code, string message)
    signal back()

    QtObject {
        id: relayObject
        function rowCount() {
            return 0;
        }
        function setAccount(service, did, handle, email, accessJwt, refreshJwt) {
        }
        function getLatest() {
        }
    }

    Frame {
        Layout.fillWidth: true
        leftPadding: 0
        topPadding: 0
        rightPadding: 10
        bottomPadding: 0

        RowLayout {
            IconButton {
                Layout.preferredWidth: AdjustedValues.b30
                Layout.preferredHeight: AdjustedValues.b30
                flat: true
                iconSource: "../images/arrow_left_single.png"
                onClicked: listDetailView.back()
            }
            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 10
                font.pointSize: AdjustedValues.f10
                text: qsTr("List")
            }
        }
    }


    ColumnLayout {
        Layout.fillWidth: true
        Layout.topMargin: 0
        Layout.leftMargin: 0
        Layout.rightMargin: 0
        Layout.bottomMargin: 5

        Label {
            text: listDetailView.listUri
        }
    }
}
