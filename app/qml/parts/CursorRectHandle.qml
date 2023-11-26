import QtQuick 2.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

Rectangle {
    id: handle
    width: 12 * AdjustedValues.ratioHalf
    height: 12 * AdjustedValues.ratioHalf
    radius: 6
    color: Material.accentColor
    signal cursorMoved(int diff_x, int diff_y)

    property bool squareMode: false
    property int cursorShape: Qt.SizeBDiagCursor

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        property int startX: 0
        property int startY: 0
        states: State {
            when: mouseArea.containsMouse
            PropertyChanges {
                target: mouseArea
                cursorShape: handle.cursorShape
            }
        }
        onPressed: (mouse) => {
                       startX = mouse.x
                       startY = mouse.y
                   }
        onPositionChanged: (mouse) => {
                               if(pressed){
                                   var dx = mouse.x - startX
                                   var dy = mouse.y - startY
                                   if(squareMode){
                                       var dx2 = Math.abs(dx)
                                       var dy2 = Math.abs(dy)
                                       if(dx2 < dy2){
                                           dy = dx2 * (dy > 0 ? 1 : -1)
                                       }else{
                                           dx = dy2 * (dx > 0 ? 1 : -1)
                                       }
                                   }
                                   cursorMoved(dx, dy)
                               }
                           }
    }
}
