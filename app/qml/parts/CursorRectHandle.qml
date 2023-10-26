import QtQuick 2.15
import QtQuick.Controls.Material 2.15

Rectangle {
    id: handle
    width: 12
    height: 12
    radius: 6
    color: Material.accentColor
    signal cursorMoved(int diff_x, int diff_y)

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
                                   cursorMoved(mouse.x - startX, mouse.y - startY)
                               }
                           }
    }
}
