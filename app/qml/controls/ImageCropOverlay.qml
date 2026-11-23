import QtQuick 2.15

import tech.relog.hagoromo.singleton 1.0

// ImageEditCanvas（showWholeImage: true）の上に重ねてクロップ範囲を編集する
Item {
    id: overlay

    property var canvas: null
    property var editModel: null

    function reset() {
        if(!editModel){
            return
        }
        var r = editModel.cropRect
        rectEditor.setRect(r.x, r.y, r.width, r.height)
    }
    function commit() {
        if(editModel){
            editModel.setCrop(rectEditor.rectX, rectEditor.rectY, rectEditor.rectWidth, rectEditor.rectHeight)
        }
        reset()
    }

    onVisibleChanged: if(visible) reset()
    Connections {
        target: overlay.editModel
        function onStateChanged() { overlay.reset() }
    }

    // クロップ範囲外を暗くする
    Rectangle {
        x: canvas ? canvas.paintedX : 0
        y: canvas ? canvas.paintedY : 0
        width: canvas ? canvas.paintedWidth : 0
        height: rectEditor.viewTop - y
        color: "#99000000"
    }
    Rectangle {
        x: canvas ? canvas.paintedX : 0
        y: rectEditor.viewBottom
        width: canvas ? canvas.paintedWidth : 0
        height: canvas ? canvas.paintedY + canvas.paintedHeight - y : 0
        color: "#99000000"
    }
    Rectangle {
        x: canvas ? canvas.paintedX : 0
        y: rectEditor.viewTop
        width: rectEditor.viewLeft - x
        height: rectEditor.viewBottom - rectEditor.viewTop
        color: "#99000000"
    }
    Rectangle {
        x: rectEditor.viewRight
        y: rectEditor.viewTop
        width: canvas ? canvas.paintedX + canvas.paintedWidth - x : 0
        height: rectEditor.viewBottom - rectEditor.viewTop
        color: "#99000000"
    }

    ImageRectEditor {
        id: rectEditor
        anchors.fill: parent
        canvas: overlay.canvas
        boundsWidth: overlay.editModel ? overlay.editModel.imageWidth : 0
        boundsHeight: overlay.editModel ? overlay.editModel.imageHeight : 0
        minLength: Math.max(8, 32 * AdjustedValues.ratio / paintedScale)
        showGrid: true
        onCommitted: overlay.commit()
    }
}
