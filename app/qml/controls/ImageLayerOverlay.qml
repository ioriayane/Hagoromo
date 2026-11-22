import QtQuick 2.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

// ImageEditCanvasの上に重ねてレイヤー（モザイク）を追加・選択・移動・リサイズする
// 何もない場所をドラッグ: 追加 / レイヤーをクリック: 選択 / レイヤーをドラッグ: 移動
Item {
    id: overlay

    property var canvas: null
    property var editModel: null
    // 新しく追加するレイヤーの設定
    property int newLayerShape: 0
    property int newLayerStrength: 5

    readonly property real paintedScale: (canvas && canvas.paintedScale > 0) ? canvas.paintedScale : 1
    readonly property real imageWidth: editModel ? editModel.imageWidth : 0
    readonly property real imageHeight: editModel ? editModel.imageHeight : 0

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
        cursorShape: Qt.CrossCursor

        property bool drawing: false
        property bool moving: false
        property point startPos: Qt.point(0, 0)
        property rect startRect: Qt.rect(0, 0, 0, 0)
        // 追加中の範囲（回転後の原寸画像座標）
        property rect band: Qt.rect(0, 0, 0, 0)

        onPressed: (mouse) => {
            if(!overlay.editModel || !overlay.canvas){
                return
            }
            var p = overlay.canvas.mapToImage(mouse.x, mouse.y)
            startPos = p
            var index = overlay.editModel.layerAt(p.x, p.y)
            if(index >= 0){
                overlay.editModel.selectedLayer = index
                startRect = overlay.editModel.selectedLayerRect
                moving = true
            }else{
                overlay.editModel.selectedLayer = -1
                band = Qt.rect(overlay.clampX(p.x), overlay.clampY(p.y), 0, 0)
                drawing = true
            }
        }
        onPositionChanged: (mouse) => {
            if(!pressed || !overlay.editModel || !overlay.canvas){
                return
            }
            var p = overlay.canvas.mapToImage(mouse.x, mouse.y)
            if(drawing){
                var x1 = overlay.clampX(Math.min(startPos.x, p.x))
                var y1 = overlay.clampY(Math.min(startPos.y, p.y))
                var x2 = overlay.clampX(Math.max(startPos.x, p.x))
                var y2 = overlay.clampY(Math.max(startPos.y, p.y))
                band = Qt.rect(x1, y1, x2 - x1, y2 - y1)
            }else if(moving){
                var x = Math.max(0, Math.min(startRect.x + p.x - startPos.x, overlay.imageWidth - startRect.width))
                var y = Math.max(0, Math.min(startRect.y + p.y - startPos.y, overlay.imageHeight - startRect.height))
                overlay.editModel.setLayerRect(overlay.editModel.selectedLayer,
                                               x, y, startRect.width, startRect.height, true)
            }
        }
        onReleased: {
            if(drawing){
                drawing = false
                // 画面上で小さすぎる範囲はクリックとみなして追加しない
                var min = 8 * AdjustedValues.ratio / overlay.paintedScale
                if(band.width >= min && band.height >= min){
                    overlay.editModel.addMosaic(band.x, band.y, band.width, band.height,
                                                overlay.newLayerShape, overlay.newLayerStrength)
                }
                band = Qt.rect(0, 0, 0, 0)
            }else if(moving){
                moving = false
                overlay.editModel.commitTransient()
            }
        }
        onCanceled: {
            drawing = false
            band = Qt.rect(0, 0, 0, 0)
            if(moving){
                moving = false
                overlay.editModel.commitTransient()
            }
        }
    }

    // 追加中の範囲
    Rectangle {
        visible: mouseArea.drawing
        x: overlay.viewX(mouseArea.band.x)
        y: overlay.viewY(mouseArea.band.y)
        width: mouseArea.band.width * overlay.paintedScale
        height: mouseArea.band.height * overlay.paintedScale
        radius: overlay.newLayerShape === 1 ? Math.min(width, height) / 2 : 0
        color: "#40ffffff"
        border.color: Material.accent
        border.width: 1
    }

    // 選択中のレイヤー
    ImageRectEditor {
        id: rectEditor
        anchors.fill: parent
        visible: overlay.editModel !== null && overlay.editModel.selectedLayer >= 0 && !mouseArea.drawing
        canvas: overlay.canvas
        boundsWidth: overlay.imageWidth
        boundsHeight: overlay.imageHeight
        frameColor: Material.accent
        onChanging: {
            overlay.editModel.setLayerRect(overlay.editModel.selectedLayer,
                                           rectX, rectY, rectWidth, rectHeight, true)
        }
        onCommitted: overlay.editModel.commitTransient()
    }
}
