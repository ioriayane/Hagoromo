import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

MouseArea {
    anchors.fill: parent
    // String[]
    // date, handle, endpoint, date, handle, endpoint, ...
    property alias model: repeater.model

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
                font.pointSize: AdjustedValues.f8
                text: qsTr("History")
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
