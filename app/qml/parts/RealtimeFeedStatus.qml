import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.realtime.realtimefeedstatuslistmodel 1.0
import tech.relog.hagoromo.realtime.realtimefeedstatusgraph 1.0
import tech.relog.hagoromo.singleton 1.0

Frame {
    id: realtimeFeedStatusFrame
    property int theme: 0

    background: Rectangle {
        radius: 3
        border.width: 1
        border.color: Material.frameColor
        color: Material.backgroundColor
    }

    RowLayout {
        spacing: 10
        RealtimeFeedStatusGraph {
            Layout.preferredWidth: paramLayout.implicitWidth * 1.5
            Layout.preferredHeight: paramLayout.implicitHeight
            visible: graphControlLabel.checked
            theme: realtimeFeedStatusFrame.theme
        }
        ColumnLayout {
            id: paramLayout
            Repeater {
                model: RealtimeFeedStatusListModel {
                    theme: realtimeFeedStatusFrame.theme
                }
                RowLayout {
                    Label {
                        font.pointSize: AdjustedValues.f8
                        text: model.name
                        color: model.useColor ? model.color : Material.foreground
                    }
                    Label {
                        Layout.minimumWidth: 50 * AdjustedValues.ratio
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                        font.pointSize: AdjustedValues.f8
                        text: model.value
                    }
                    Label {
                        font.pointSize: AdjustedValues.f6
                        text: model.unit
                    }
                }
            }
            Label {
                id: graphControlLabel
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                font.pointSize: AdjustedValues.f8
                text: (checked ? "■" : "□")+"History"
                color: Material.color(Material.Blue)
                property bool checked: false
                MouseArea {
                    anchors.fill: parent
                    onClicked: graphControlLabel.checked = !graphControlLabel.checked
                }
            }
        }
    }
}
