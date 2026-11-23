import QtQuick 2.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

// ImageEditCanvasの上で矩形（回転後の原寸画像座標）を移動・リサイズする枠とハンドル
Item {
    id: editor

    property var canvas: null
    // 編集中の矩形（回転後の原寸画像座標）
    property real rectX: 0
    property real rectY: 0
    property real rectWidth: 0
    property real rectHeight: 0
    // 移動・リサイズできる範囲（回転後の原寸画像座標）
    property real boundsWidth: 0
    property real boundsHeight: 0
    property bool showGrid: false
    property color frameColor: "white"
    // 縦横比を保ってリサイズする（四隅のハンドルのみ）
    property bool keepAspect: false
    readonly property bool dragging: moveArea.pressed || handleRepeater.pressedCount > 0

    // ドラッグ中に矩形が変わったとき
    signal changing()
    // ドラッグを終えたとき
    signal committed()

    readonly property real paintedScale: (canvas && canvas.paintedScale > 0) ? canvas.paintedScale : 1
    // 表示上の最小サイズ（画像座標）
    property real minLength: Math.max(4, 24 * AdjustedValues.ratio / paintedScale)
    readonly property real handleSize: 14 * AdjustedValues.ratio

    // アイテム座標での矩形
    readonly property real viewLeft: canvas ? canvas.paintedX + (rectX - canvas.regionX) * paintedScale : 0
    readonly property real viewTop: canvas ? canvas.paintedY + (rectY - canvas.regionY) * paintedScale : 0
    readonly property real viewRight: viewLeft + rectWidth * paintedScale
    readonly property real viewBottom: viewTop + rectHeight * paintedScale

    function setRect(x, y, width, height) {
        rectX = x
        rectY = y
        rectWidth = width
        rectHeight = height
    }

    // 枠と三分割線
    Rectangle {
        id: frame
        x: editor.viewLeft
        y: editor.viewTop
        width: editor.viewRight - editor.viewLeft
        height: editor.viewBottom - editor.viewTop
        color: "transparent"
        border.color: editor.frameColor
        border.width: 1
        Repeater {
            model: editor.showGrid ? 2 : 0
            Rectangle {
                required property int index
                x: frame.width * (index + 1) / 3
                width: 1
                height: frame.height
                color: "#80ffffff"
            }
        }
        Repeater {
            model: editor.showGrid ? 2 : 0
            Rectangle {
                required property int index
                y: frame.height * (index + 1) / 3
                width: frame.width
                height: 1
                color: "#80ffffff"
            }
        }
        // 範囲の移動
        RectDragArea {
            id: moveArea
            anchors.fill: parent
            moveAll: true
            cursorShape: Qt.SizeAllCursor
        }
    }

    // 四隅と各辺のハンドル
    Repeater {
        id: handleRepeater
        property int pressedCount: 0
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
            visible: !editor.keepAspect || (modelData.px !== 0.5 && modelData.py !== 0.5)
            x: editor.viewLeft + (editor.viewRight - editor.viewLeft) * modelData.px - width / 2
            y: editor.viewTop + (editor.viewBottom - editor.viewTop) * modelData.py - height / 2
            width: editor.handleSize
            height: editor.handleSize
            color: "white"
            border.color: Material.accent
            border.width: 1
            RectDragArea {
                anchors.fill: parent
                anchors.margins: -4 * AdjustedValues.ratio
                edgeLeft: parent.modelData.l
                edgeTop: parent.modelData.t
                edgeRight: parent.modelData.r
                edgeBottom: parent.modelData.b
                cursorShape: parent.modelData.cursor
                onPressedChanged: handleRepeater.pressedCount += (pressed ? 1 : -1)
            }
        }
    }

    component RectDragArea: MouseArea {
        property bool moveAll: false
        property bool edgeLeft: false
        property bool edgeTop: false
        property bool edgeRight: false
        property bool edgeBottom: false

        property point startPos: Qt.point(0, 0)
        property rect startRect: Qt.rect(0, 0, 0, 0)

        preventStealing: true
        onPressed: (mouse) => {
            startPos = mapToItem(editor, mouse.x, mouse.y)
            startRect = Qt.rect(editor.rectX, editor.rectY, editor.rectWidth, editor.rectHeight)
        }
        onPositionChanged: (mouse) => {
            if(!pressed){
                return
            }
            var p = mapToItem(editor, mouse.x, mouse.y)
            var dx = (p.x - startPos.x) / editor.paintedScale
            var dy = (p.y - startPos.y) / editor.paintedScale
            var x1 = startRect.x
            var y1 = startRect.y
            var x2 = startRect.x + startRect.width
            var y2 = startRect.y + startRect.height
            var min = Math.min(editor.minLength, editor.boundsWidth, editor.boundsHeight)
            if(editor.keepAspect && !moveAll && (edgeLeft || edgeRight) && (edgeTop || edgeBottom) &&
                    startRect.width > 0 && startRect.height > 0){
                // 反対側の角を固定して、横方向の移動量で大きさを決める
                var ratio = startRect.width / startRect.height
                var ax = edgeRight ? startRect.x : startRect.x + startRect.width
                var ay = edgeBottom ? startRect.y : startRect.y + startRect.height
                var w = edgeRight ? startRect.width + dx : startRect.width - dx
                w = Math.max(min * Math.max(1, ratio), w)
                var h = w / ratio
                var max_w = edgeRight ? editor.boundsWidth - ax : ax
                var max_h = edgeBottom ? editor.boundsHeight - ay : ay
                if(w > max_w){
                    w = max_w
                    h = w / ratio
                }
                if(h > max_h){
                    h = max_h
                    w = h * ratio
                }
                editor.setRect(edgeRight ? ax : ax - w, edgeBottom ? ay : ay - h, w, h)
                editor.changing()
                return
            }
            if(moveAll){
                x1 = Math.max(0, Math.min(startRect.x + dx, editor.boundsWidth - startRect.width))
                y1 = Math.max(0, Math.min(startRect.y + dy, editor.boundsHeight - startRect.height))
                x2 = x1 + startRect.width
                y2 = y1 + startRect.height
            }
            if(edgeLeft){
                x1 = Math.max(0, Math.min(x1 + dx, x2 - min))
            }
            if(edgeRight){
                x2 = Math.min(editor.boundsWidth, Math.max(x2 + dx, x1 + min))
            }
            if(edgeTop){
                y1 = Math.max(0, Math.min(y1 + dy, y2 - min))
            }
            if(edgeBottom){
                y2 = Math.min(editor.boundsHeight, Math.max(y2 + dy, y1 + min))
            }
            editor.setRect(x1, y1, x2 - x1, y2 - y1)
            editor.changing()
        }
        onReleased: editor.committed()
    }
}
