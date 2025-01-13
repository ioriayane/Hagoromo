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
                    Layout.minimumWidth: 70 * AdjustedValues.ratio
                    font.pointSize: AdjustedValues.f8
                    text: model.name
                }
                Label {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                    font.pointSize: AdjustedValues.f8
                    text: model.value
                }
                Label {
                    font.pointSize: AdjustedValues.f8
                    text: model.unit
                }
            }
        }
    }
}
