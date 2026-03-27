import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import tech.relog.hagoromo.singleton 1.0

Frame {
    id: operationProgressFrame

    property string headerText: ""
    property string message: ""
    property bool fixedWidth: false

    contentWidth: progressLayout.width
    contentHeight: progressLayout.height
    background: Rectangle {
        radius: 3
        border.width: 1
        border.color: Material.frameColor
        color: Material.backgroundColor
    }
    ColumnLayout {
        id: progressLayout
        width: operationProgressFrame.fixedWidth
               ? 300 * AdjustedValues.ratio
               : Math.max(300 * AdjustedValues.ratio, headerLabel.implicitWidth, progressLabel.implicitWidth)
        Label {
            id: headerLabel
            Layout.fillWidth: true
            font.pointSize: AdjustedValues.f10
            elide: Text.ElideRight
            text: operationProgressFrame.headerText.split("\n")[0]
            clip: true
        }
        ProgressBar {
            Layout.fillWidth: true
            indeterminate: true
        }
        Label {
            id: progressLabel
            Layout.fillWidth: true
            font.pointSize: AdjustedValues.f8
            text: operationProgressFrame.message
            color: Material.theme === Material.Dark ? Material.foreground : "white"
            visible: operationProgressFrame.message.length > 0
        }
    }
}
