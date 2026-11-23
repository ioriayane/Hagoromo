import QtQuick 2.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.controls.imageeditmodel 1.0
import tech.relog.hagoromo.singleton 1.0

// ImageEditCanvasの上に重ねてレイヤー（モザイク・描画・スタンプ）を追加・選択・移動・リサイズする
// 何もない場所をドラッグ: 追加 / レイヤーをクリック: 選択 / レイヤーをドラッグ: 移動
// allowSelect=falseのときは既存のレイヤーをつかまずに常に追加する（折れ線を描くときなど）
Item {
    id: overlay

    property var canvas: null
    property var editModel: null
    // "mosaic" / "draw" / "stamp"
    property string mode: "mosaic"
    // 既存のレイヤーを選択・移動・リサイズできる
    property bool allowSelect: true
    // ドラッグで新しいレイヤーを追加できる
    property bool allowCreate: true
    // 新しく追加するモザイクの設定
    property int newLayerShape: 0
    property int newLayerStrength: 5
    // 新しく追加する描画の設定
    property int drawType: ImageEditModel.TypeLine
    property color drawColor: "red"
    property int drawLineWidth: 3
    property bool drawFill: false

    readonly property real paintedScale: (canvas && canvas.paintedScale > 0) ? canvas.paintedScale : 1
    readonly property real imageWidth: editModel ? editModel.imageWidth : 0
    readonly property real imageHeight: editModel ? editModel.imageHeight : 0
    readonly property bool selectedLine: editModel !== null && editModel.selectedLayer >= 0 &&
                                         (editModel.selectedLayerType === ImageEditModel.TypeLine ||
                                          editModel.selectedLayerType === ImageEditModel.TypeArrow)
    // 線をつかむときの余裕（画像座標）
    readonly property real tolerance: 6 * AdjustedValues.ratio / paintedScale

    function viewX(image_x) {
        return canvas ? canvas.paintedX + (image_x - canvas.regionX) * paintedScale : 0
    }
    function viewY(image_y) {
        return canvas ? canvas.paintedY + (image_y - canvas.regionY) * paintedScale : 0
    }
    function clampX(x) {
        return Math.max(0, Math.min(x, imageWidth))
    }
    function clampY(y) {
        return Math.max(0, Math.min(y, imageHeight))
    }
    function reset() {
        if(!editModel || editModel.selectedLayer < 0){
            return
        }
        var r = editModel.selectedLayerRect
        rectEditor.setRect(r.x, r.y, r.width, r.height)
    }

    onVisibleChanged: if(visible) reset()
    Connections {
        target: overlay.editModel
        function onStateChanged() { overlay.reset() }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        preventStealing: true
        cursorShape: overlay.allowCreate && overlay.mode !== "stamp" ? Qt.CrossCursor : Qt.ArrowCursor

        property bool drawing: false
        property bool moving: false
        property point startPos: Qt.point(0, 0)
        property rect startRect: Qt.rect(0, 0, 0, 0)
        // 追加中のモザイクの範囲（回転後の原寸画像座標）
        property rect band: Qt.rect(0, 0, 0, 0)
        // 描画中のレイヤー
        property int drawingIndex: -1
        property var drawingPoints: []

        function finishDrawing() {
            if(drawing && overlay.mode === "draw"){
                overlay.editModel.commitTransient()
                if(!overlay.allowSelect){
                    // 選択できないモードでは描き終えたら選択を外す（次も続けて描けるように）
                    overlay.editModel.selectedLayer = -1
                }
            }
            drawing = false
            drawingIndex = -1
            drawingPoints = []
            band = Qt.rect(0, 0, 0, 0)
        }

        onPressed: (mouse) => {
            if(!overlay.editModel || !overlay.canvas){
                return
            }
            var p = overlay.canvas.mapToImage(mouse.x, mouse.y)
            startPos = p
            var category = ImageEditModel.CategoryMosaic
            if(overlay.mode === "draw"){
                category = ImageEditModel.CategoryDrawing
            }else if(overlay.mode === "stamp"){
                category = ImageEditModel.CategoryStamp
            }
            var index = overlay.allowSelect ? overlay.editModel.layerAt(p.x, p.y, overlay.tolerance, category) : -1
            if(index >= 0){
                overlay.editModel.selectedLayer = index
                startRect = overlay.editModel.selectedLayerRect
                moving = true
                return
            }
            overlay.editModel.selectedLayer = -1
            if(!overlay.allowCreate || overlay.mode === "stamp"){
                // スタンプはパネルから追加する
                return
            }
            drawing = true
            if(overlay.mode === "mosaic"){
                band = Qt.rect(overlay.clampX(p.x), overlay.clampY(p.y), 0, 0)
            }else{
                // 描画中の変更は描き終わりで1回分の履歴にする
                overlay.editModel.beginTransaction()
                drawingIndex = -1
                if(overlay.drawType === ImageEditModel.TypeFreehand){
                    drawingPoints = [Qt.point(p.x, p.y)]
                    drawingIndex = overlay.editModel.addFreehand(drawingPoints, overlay.drawColor,
                                                                 overlay.drawLineWidth)
                }
            }
        }
        onPositionChanged: (mouse) => {
            if(!pressed || !overlay.editModel || !overlay.canvas){
                return
            }
            var p = overlay.canvas.mapToImage(mouse.x, mouse.y)
            if(moving){
                var x = Math.max(0, Math.min(startRect.x + p.x - startPos.x, overlay.imageWidth - startRect.width))
                var y = Math.max(0, Math.min(startRect.y + p.y - startPos.y, overlay.imageHeight - startRect.height))
                overlay.editModel.setLayerRect(overlay.editModel.selectedLayer,
                                               x, y, startRect.width, startRect.height, true)
                return
            }
            if(!drawing){
                return
            }
            var x1 = overlay.clampX(Math.min(startPos.x, p.x))
            var y1 = overlay.clampY(Math.min(startPos.y, p.y))
            var x2 = overlay.clampX(Math.max(startPos.x, p.x))
            var y2 = overlay.clampY(Math.max(startPos.y, p.y))
            if(overlay.mode === "mosaic"){
                band = Qt.rect(x1, y1, x2 - x1, y2 - y1)
                return
            }
            var end = Qt.point(overlay.clampX(p.x), overlay.clampY(p.y))
            if(overlay.drawType === ImageEditModel.TypeFreehand){
                // 画面上で2px以上動いたら点を追加する
                var last = drawingPoints[drawingPoints.length - 1]
                var min = 2 * AdjustedValues.ratio / overlay.paintedScale
                if(Math.abs(end.x - last.x) < min && Math.abs(end.y - last.y) < min){
                    return
                }
                var points = drawingPoints.slice()
                points.push(end)
                drawingPoints = points
                overlay.editModel.setLayerPoints(drawingIndex, drawingPoints, true)
            }else if(overlay.drawType === ImageEditModel.TypeLine ||
                     overlay.drawType === ImageEditModel.TypeArrow){
                if(drawingIndex < 0){
                    if(overlay.drawType === ImageEditModel.TypeArrow){
                        drawingIndex = overlay.editModel.addArrow(startPos.x, startPos.y, end.x, end.y,
                                                                  overlay.drawColor, overlay.drawLineWidth)
                    }else{
                        drawingIndex = overlay.editModel.addLine(startPos.x, startPos.y, end.x, end.y,
                                                                 overlay.drawColor, overlay.drawLineWidth)
                    }
                }else{
                    overlay.editModel.setLayerPoints(drawingIndex, [startPos, end], true)
                }
            }else{
                if(drawingIndex < 0){
                    drawingIndex = overlay.editModel.addShape(overlay.drawType, x1, y1, x2 - x1, y2 - y1,
                                                              overlay.drawColor, overlay.drawLineWidth,
                                                              overlay.drawFill)
                }else{
                    overlay.editModel.setLayerRect(drawingIndex, x1, y1, x2 - x1, y2 - y1, true)
                }
            }
        }
        onReleased: {
            if(moving){
                moving = false
                overlay.editModel.commitTransient()
            }else if(drawing && overlay.mode === "mosaic"){
                // 画面上で小さすぎる範囲はクリックとみなして追加しない
                var min = 8 * AdjustedValues.ratio / overlay.paintedScale
                if(band.width >= min && band.height >= min){
                    overlay.editModel.addMosaic(band.x, band.y, band.width, band.height,
                                                overlay.newLayerShape, overlay.newLayerStrength)
                }
                finishDrawing()
            }else{
                finishDrawing()
            }
        }
        onCanceled: {
            if(moving){
                moving = false
                overlay.editModel.commitTransient()
            }
            finishDrawing()
        }
    }

    // 追加中のモザイクの範囲
    Rectangle {
        visible: mouseArea.drawing && overlay.mode === "mosaic"
        x: overlay.viewX(mouseArea.band.x)
        y: overlay.viewY(mouseArea.band.y)
        width: mouseArea.band.width * overlay.paintedScale
        height: mouseArea.band.height * overlay.paintedScale
        radius: overlay.newLayerShape === 1 ? Math.min(width, height) / 2 : 0
        color: "#40ffffff"
        border.color: Material.accent
        border.width: 1
    }

    // 選択中のレイヤー（直線以外）
    ImageRectEditor {
        id: rectEditor
        anchors.fill: parent
        visible: overlay.allowSelect && overlay.editModel !== null && overlay.editModel.selectedLayer >= 0 &&
                 !overlay.selectedLine && !mouseArea.drawing
        canvas: overlay.canvas
        boundsWidth: overlay.imageWidth
        boundsHeight: overlay.imageHeight
        frameColor: Material.accent
        keepAspect: overlay.editModel !== null && overlay.editModel.selectedLayerType === ImageEditModel.TypeStamp
        onChanging: {
            overlay.editModel.setLayerRect(overlay.editModel.selectedLayer,
                                           rectX, rectY, rectWidth, rectHeight, true)
        }
        onCommitted: overlay.editModel.commitTransient()
    }

    // 選択中の直線の端点
    Repeater {
        // ドラッグ中にデリゲートが作り直されないように個数で指定する
        model: overlay.allowSelect && overlay.selectedLine && !mouseArea.drawing ? 2 : 0
        Rectangle {
            id: endPoint
            required property int index
            readonly property var point: overlay.selectedLine ? overlay.editModel.selectedLayerPoints[index] : Qt.point(0, 0)
            x: overlay.viewX(point.x) - width / 2
            y: overlay.viewY(point.y) - height / 2
            width: 14 * AdjustedValues.ratio
            height: width
            radius: width / 2
            // 矢印の先端などが隠れないように半透明にする
            color: "#80ffffff"
            border.color: Material.accent
            border.width: 1
            MouseArea {
                anchors.fill: parent
                anchors.margins: -4 * AdjustedValues.ratio
                preventStealing: true
                cursorShape: Qt.SizeAllCursor
                onPositionChanged: (mouse) => {
                    if(!pressed){
                        return
                    }
                    var m = mapToItem(overlay, mouse.x, mouse.y)
                    var p = overlay.canvas.mapToImage(m.x, m.y)
                    var points = overlay.editModel.selectedLayerPoints
                    points[endPoint.index] = Qt.point(overlay.clampX(p.x), overlay.clampY(p.y))
                    overlay.editModel.setLayerPoints(overlay.editModel.selectedLayer, points, true)
                }
                onReleased: overlay.editModel.commitTransient()
            }
        }
    }
}
