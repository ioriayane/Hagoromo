import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.realtime.realtimefeedstatuslistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

Frame {
    background: Rectangle {
        radius: 3
        border.width: 1
        border.color: Material.frameColor
        color: Material.backgroundColor
    }

    ColumnLayout {
        Repeater {
            model: RealtimeFeedStatusListModel {
            }
            RowLayout {
                Label {
                    text: model.name
                }
                Label {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                    text: model.value
                }
                Label {
                    text: model.unit
                }
            }
        }
    }
}
