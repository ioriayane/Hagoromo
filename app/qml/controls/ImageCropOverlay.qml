import QtQuick 2.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

// ImageEditCanvas（showWholeImage: true）の上に重ねてクロップ範囲を編集する
Item {
    id: overlay

    property var canvas: null
    property var editModel: null

    // 編集中のクロップ範囲（回転後の原寸画像座標）
    property real cropX: 0
    property real cropY: 0
    property real cropWidth: 0
    property real cropHeight: 0

    readonly property real imageWidth: editModel ? editModel.imageWidth : 0
    readonly property real imageHeight: editModel ? editModel.imageHeight : 0
    readonly property real paintedScale: (canvas && canvas.paintedScale > 0) ? canvas.paintedScale : 1
    // 表示上の最小サイズ（画像座標）
    readonly property real minLength: Math.max(8, 32 * AdjustedValues.ratio / paintedScale)
    readonly property real handleSize: 14 * AdjustedValues.ratio

    // アイテム座標でのクロップ範囲
    readonly property real viewLeft: canvas ? canvas.paintedX + cropX * paintedScale : 0
    readonly property real viewTop: canvas ? canvas.paintedY + cropY * paintedScale : 0
    readonly property real viewRight: viewLeft + cropWidth * paintedScale
    readonly property real viewBottom: viewTop + cropHeight * paintedScale

    function reset() {
        if(!editModel){
            return
        }
        var r = editModel.cropRect
        cropX = r.x
        cropY = r.y
        cropWidth = r.width
        cropHeight = r.height
    }
    function commit() {
        if(editModel){
            editModel.setCrop(cropX, cropY, cropWidth, cropHeight)
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
        height: overlay.viewTop - y
        color: "#99000000"
    }
    Rectangle {
        x: canvas ? canvas.paintedX : 0
        y: overlay.viewBottom
        width: canvas ? canvas.paintedWidth : 0
        height: canvas ? canvas.paintedY + canvas.paintedHeight - y : 0
        color: "#99000000"
    }
    Rectangle {
        x: canvas ? canvas.paintedX : 0
        y: overlay.viewTop
        width: overlay.viewLeft - x
        height: overlay.viewBottom - overlay.viewTop
        color: "#99000000"
    }
    Rectangle {
        x: overlay.viewRight
        y: overlay.viewTop
        width: canvas ? canvas.paintedX + canvas.paintedWidth - x : 0
        height: overlay.viewBottom - overlay.viewTop
        color: "#99000000"
    }

    // 枠と三分割線
    Rectangle {
        id: frame
        x: overlay.viewLeft
        y: overlay.viewTop
        width: overlay.viewRight - overlay.viewLeft
        height: overlay.viewBottom - overlay.viewTop
        color: "transparent"
        border.color: "white"
        border.width: 1
        Repeater {
            model: 2
            Rectangle {
                required property int index
                x: frame.width * (index + 1) / 3
                width: 1
                height: frame.height
                color: "#80ffffff"
            }
        }
        Repeater {
            model: 2
            Rectangle {
                required property int index
                y: frame.height * (index + 1) / 3
                width: frame.width
                height: 1
                color: "#80ffffff"
            }
        }
        // 範囲の移動
        CropDragArea {
            anchors.fill: parent
            moveAll: true
            cursorShape: Qt.SizeAllCursor
        }
    }

    // 四隅と各辺のハンドル
    Repeater {
        model: [
            { l: true,  t: true,  r: false, b: false, px: 0,   py: 0,   cursor: Qt.SizeFDiagCursor },
            { l: false, t: true,  r: false, b: false, px: 0.5, py: 0,   cursor: Qt.SizeVerCursor },
            { l: false, t: true,  r: true,  b: false, px: 1,   py: 0,   cursor: Qt.SizeBDiagCursor },
            { l: false, t: false, r: true,  b: false, px: 1,   py: 0.5, cursor: Qt.SizeHorCursor },
            { l: false, t: false, r: true,  b: true,  px: 1,   py: 1,   cursor: Qt.SizeFDiagCursor },
            { l: false, t: false, r: false, b: true,  px: 0.5, py: 1,   cursor: Qt.SizeVerCursor },
            { l: true,  t: false, r: false, b: true,  px: 0,   py: 1,   cursor: Qt.SizeBDiagCursor },
            { l: true,  t: false, r: false, b: false, px: 0,   py: 0.5, cursor: Qt.SizeHorCursor }
        ]
        Rectangle {
            required property var modelData
            x: overlay.viewLeft + (overlay.viewRight - overlay.viewLeft) * modelData.px - width / 2
            y: overlay.viewTop + (overlay.viewBottom - overlay.viewTop) * modelData.py - height / 2
            width: overlay.handleSize
            height: overlay.handleSize
            color: "white"
            border.color: Material.accent
            border.width: 1
            CropDragArea {
                anchors.fill: parent
                anchors.margins: -4 * AdjustedValues.ratio
                edgeLeft: parent.modelData.l
                edgeTop: parent.modelData.t
                edgeRight: parent.modelData.r
                edgeBottom: parent.modelData.b
                cursorShape: parent.modelData.cursor
            }
        }
    }

    component CropDragArea: MouseArea {
        property bool moveAll: false
        property bool edgeLeft: false
        property bool edgeTop: false
        property bool edgeRight: false
        property bool edgeBottom: false

        property point startPos: Qt.point(0, 0)
        property rect startRect: Qt.rect(0, 0, 0, 0)

        preventStealing: true
        onPressed: (mouse) => {
            startPos = mapToItem(overlay, mouse.x, mouse.y)
            startRect = Qt.rect(overlay.cropX, overlay.cropY, overlay.cropWidth, overlay.cropHeight)
        }
        onPositionChanged: (mouse) => {
            if(!pressed){
                return
            }
            var p = mapToItem(overlay, mouse.x, mouse.y)
            var dx = (p.x - startPos.x) / overlay.paintedScale
            var dy = (p.y - startPos.y) / overlay.paintedScale
            var x1 = startRect.x
            var y1 = startRect.y
            var x2 = startRect.x + startRect.width
            var y2 = startRect.y + startRect.height
            var min = Math.min(overlay.minLength, overlay.imageWidth, overlay.imageHeight)
            if(moveAll){
                x1 = Math.max(0, Math.min(startRect.x + dx, overlay.imageWidth - startRect.width))
                y1 = Math.max(0, Math.min(startRect.y + dy, overlay.imageHeight - startRect.height))
                x2 = x1 + startRect.width
                y2 = y1 + startRect.height
            }
            if(edgeLeft){
                x1 = Math.max(0, Math.min(x1 + dx, x2 - min))
            }
            if(edgeRight){
                x2 = Math.min(overlay.imageWidth, Math.max(x2 + dx, x1 + min))
            }
            if(edgeTop){
                y1 = Math.max(0, Math.min(y1 + dy, y2 - min))
            }
            if(edgeBottom){
                y2 = Math.min(overlay.imageHeight, Math.max(y2 + dy, y1 + min))
            }
            overlay.cropX = x1
            overlay.cropY = y1
            overlay.cropWidth = x2 - x1
            overlay.cropHeight = y2 - y1
        }
        onReleased: overlay.commit()
    }
}
