import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import tech.relog.hagoromo.singleton 1.0

Frame {
    id: operationProgressFrame

    property string headerText: ""
    property bool primaryRunning: false
    property string primaryProgressMessage: ""
    property bool secondaryRunning: false
    property string secondaryProgressMessage: ""

    contentWidth: progressLayout.width
    contentHeight: progressLayout.height
    visible: (primaryRunning && primaryProgressMessage.length > 0) ||
             (secondaryRunning && secondaryProgressMessage.length > 0)
    background: Rectangle {
        radius: 3
        border.width: 1
        border.color: Material.frameColor
        color: Material.backgroundColor
    }
    ColumnLayout {
        id: progressLayout
        width: 300 * AdjustedValues.ratio
        Label {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width
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
            Layout.maximumWidth: parent.width
            font.pointSize: AdjustedValues.f8
            text: operationProgressFrame.primaryProgressMessage.length > 0 ?
                      operationProgressFrame.primaryProgressMessage :
                      (operationProgressFrame.secondaryRunning ?
                           operationProgressFrame.secondaryProgressMessage :
                           operationProgressFrame.primaryProgressMessage)
            color: Material.theme === Material.Dark ? Material.foreground : "white"
        }
    }
}
