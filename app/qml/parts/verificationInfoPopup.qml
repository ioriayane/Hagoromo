import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

MouseArea {
    anchors.fill: parent
    // String[]
    // displayName, handle, date, displayName, handle, date, ...
    property alias model: repeater.model
    property string verificationState: "none"
    property string displayName: ""
    property int layoutMaximumWidth: 200

    onClicked: {
        if(repeater.count > 0){
            popup.open()
        }
    }

    Popup {
        id: popup
        ColumnLayout {
            spacing: 3
            Label {
                font.pointSize: AdjustedValues.f10
                font.bold: true
                text: (verificationState === "verifier" ?
                          qsTr("%1 is a trusted verifier") :
                          qsTr("%1 is verified")).replace("%1", displayName)
            }
            Label {
                Layout.maximumWidth: layoutMaximumWidth
                font.pointSize: AdjustedValues.f8
                text: (verificationState === "verifier" ?
                          qsTr("Accounts with a scalloped blue check mark can verify others. These trusted verifiers are selected by Bluesky.") :
                          qsTr("This account has a checkmark because it's been verified by trusted sources."))
                wrapMode: Text.WrapAnywhere
            }
            Label {
                font.pointSize: AdjustedValues.f8
                color: Material.color(Material.Grey)
                text: qsTr("Verified by:")
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Material.color(Material.Grey)
            }
            GridLayout {
                Layout.topMargin: 2
                columns: 3
                Repeater {
                    id: repeater
                    Label {
                        font.pointSize: AdjustedValues.f8
                        text: modelData
                    }
                }
            }
        }
    }
}
