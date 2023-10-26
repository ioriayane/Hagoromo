import QtQuick 2.15
import QtQuick.Controls.Material 2.15

Item {
    id: cursorParentRect

    property alias cursorX: cursorRect.x
    property alias cursorY: cursorRect.y
    property alias cursorWidth: cursorRect.width
    property alias cursorHeight: cursorRect.height

    Rectangle {
        id: topRect
        color: "#22ffffff"
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: cursorRect.top
    }
    Rectangle {
        color: "#22ffffff"
        anchors.top: topRect.bottom
        anchors.left: parent.left
        anchors.right: cursorRect.left
        anchors.bottom: bottomRect.top
    }
    Rectangle {
        color: "#22ffffff"
        anchors.top: topRect.bottom
        anchors.left: cursorRect.right
        anchors.right: parent.right
        anchors.bottom: bottomRect.top
    }
    Rectangle {
        id: bottomRect
        color: "#22ffffff"
        anchors.top: cursorRect.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }


    Rectangle {
        id: cursorRect
        x: 0
        y: 0
        width: parent.width
        height: parent.height
        border.color: Material.accentColor
        border.width: 1
        color: "transparent"


        function updateRect(diff_x, diff_y, diff_w, diff_h){
            var new_x = cursorRect.x + diff_x
            var new_y = cursorRect.y + diff_y
            var new_w = cursorRect.width + diff_w
            var new_h = cursorRect.height + diff_h
            if(new_x < 0){
                if(diff_w > 0){
                    new_w += new_x  // はみ出た分を引く
                }else{
                    new_w = cursorRect.width
                }
                new_x = 0
            }else if((new_x + new_w) > cursorParentRect.width) {
                if(diff_x > 0){
                    new_x = cursorRect.x
                    new_w = cursorRect.width
                }else{
                    new_w = cursorParentRect.width - new_x
                }
            }
            if(new_y < 0){
                if(diff_h > 0){
                    new_h += new_y
                }else{
                    new_h = cursorRect.height
                }
                new_y = 0
            }else if((new_y + new_h) > cursorParentRect.height) {
                if(diff_y > 0){
                    new_y = cursorRect.y
                    new_h = cursorRect.height
                }else{
                    new_h = cursorParentRect.height - new_y
                }
            }
            if(new_w >= 50){
                cursorRect.x = new_x
                cursorRect.width = new_w
            }
            if(new_h >= 50){
                cursorRect.y = new_y
                cursorRect.height = new_h
            }
        }

        MouseArea {
            id: dragArea
            anchors.fill: parent
            property int startX: 0
            property int startY: 0
            onPressed: (mouse) => {
                           startX = mouse.x
                           startY = mouse.y
                       }
            onPositionChanged: (mouse) => {
                                   if(pressed){
                                       cursorRect.updateRect(mouse.x - startX, mouse.y - startY, 0, 0)
                                   }
                               }
        }

        CursorRectHandle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: -1 * width / 2
            anchors.topMargin: -1 * width / 2
            cursorShape: Qt.SizeFDiagCursor
            onCursorMoved: (diff_x, diff_y) => cursorRect.updateRect(diff_x, diff_y, -1 * diff_x, -1 * diff_y)
        }
        CursorRectHandle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: -1 * width / 2
            anchors.topMargin: -1 * width / 2
            cursorShape: Qt.SizeBDiagCursor
            onCursorMoved: (diff_x, diff_y) => cursorRect.updateRect(0, diff_y, diff_x, -1 * diff_y)
        }
        CursorRectHandle {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: -1 * width / 2
            anchors.bottomMargin: -1 * width / 2
            cursorShape: Qt.SizeBDiagCursor
            onCursorMoved: (diff_x, diff_y) => cursorRect.updateRect(diff_x, 0, -1 * diff_x, diff_y)
        }
        CursorRectHandle {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: -1 * width / 2
            anchors.bottomMargin: -1 * width / 2
            cursorShape: Qt.SizeFDiagCursor
            onCursorMoved: (diff_x, diff_y) => cursorRect.updateRect(0, 0, diff_x, diff_y)
        }
    }
}
