import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtQuick.Dialogs as QD

import tech.relog.hagoromo.controls.imageeditmodel 1.0
import tech.relog.hagoromo.controls.imageeditcanvas 1.0
import tech.relog.hagoromo.controls.stamphistorymodel 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../compat"

Dialog {
    id: imageEditDialog
    parent: Overlay.overlay
    modal: true
    closePolicy: Popup.NoAutoClose

    readonly property real outerMargin: 20 * AdjustedValues.ratio
    x: outerMargin
    y: outerMargin
    width: parent ? parent.width - outerMargin * 2 : 800
    height: parent ? parent.height - outerMargin * 2 : 600

    property string embedImage: ""
    property string embedAlt: ""
    // 編集して保存した画像のURL（編集していない場合は空）
    property string editedImage: ""
    // 選択中のツール: "" / "crop" / "mosaic" / "draw" / "stamp"
    property string tool: ""
    readonly property bool cropMode: tool === "crop"
    // 新しく追加するモザイクの設定
    property int mosaicShape: ImageEditModel.ShapeRectangle
    property int mosaicStrength: 5
    // 新しく追加する描画の設定
    property int drawType: ImageEditModel.TypeLine
    property color drawColor: "#f44336"
    property int drawLineWidth: 3
    property bool drawFill: false
    readonly property bool drawingSelected: imageEditModel.selectedLayer >= 0 &&
                                            imageEditModel.selectedLayerType !== ImageEditModel.TypeMosaic &&
                                            imageEditModel.selectedLayerType !== ImageEditModel.TypeStamp
    readonly property bool stampSelected: imageEditModel.selectedLayer >= 0 &&
                                          imageEditModel.selectedLayerType === ImageEditModel.TypeStamp
    // 選択中の描画、または新しく追加する描画の設定
    readonly property int currentDrawType: drawingSelected ? imageEditModel.selectedLayerType : drawType
    readonly property color currentDrawColor: drawingSelected ? imageEditModel.selectedLayerColor : drawColor
    readonly property int currentDrawLineWidth: drawingSelected ? imageEditModel.selectedLayerLineWidth : drawLineWidth
    readonly property bool currentDrawFill: drawingSelected ? imageEditModel.selectedLayerFill : drawFill

    function toggleTool(name) {
        imageEditDialog.tool = (imageEditDialog.tool === name) ? "" : name
    }
    function currentMosaicShape() {
        return imageEditModel.selectedLayer >= 0 ? imageEditModel.selectedLayerShape : imageEditDialog.mosaicShape
    }
    function setMosaicShape(shape) {
        imageEditDialog.mosaicShape = shape
        if(imageEditModel.selectedLayer >= 0){
            imageEditModel.setLayerShape(imageEditModel.selectedLayer, shape)
        }
    }
    function setMosaicStrength(strength, transient) {
        imageEditDialog.mosaicStrength = strength
        if(imageEditModel.selectedLayer >= 0){
            imageEditModel.setLayerStrength(imageEditModel.selectedLayer, strength, transient)
        }
    }

    function setDrawType(type) {
        imageEditDialog.drawType = type
        // 次のドラッグで新しく描けるように選択を解除する
        imageEditModel.selectedLayer = -1
    }
    function setDrawColor(color) {
        imageEditDialog.drawColor = color
        if(imageEditDialog.drawingSelected){
            imageEditModel.setLayerColor(imageEditModel.selectedLayer, color)
        }
    }
    function setDrawLineWidth(width, transient) {
        imageEditDialog.drawLineWidth = width
        if(imageEditDialog.drawingSelected){
            imageEditModel.setLayerLineWidth(imageEditModel.selectedLayer, width, transient)
        }
    }
    function setDrawFill(fill) {
        imageEditDialog.drawFill = fill
        if(imageEditDialog.drawingSelected){
            imageEditModel.setLayerFill(imageEditModel.selectedLayer, fill)
        }
    }

    // 履歴の画像を表示範囲（クロップ後）に貼り付ける
    // center_x, center_yを省略したときは表示範囲の中央
    function placeStamp(stamp_id, center_x, center_y) {
        var source = stampHistoryModel.source(stamp_id)
        var r = imageEditModel.cropRect
        var length = Math.min(r.width, r.height) * 0.3
        if(center_x === undefined || center_y === undefined){
            // 連続で貼り付けたときに重ならないように少しずらす
            var p = imageEditModel.findStampPosition(r.x + r.width / 2, r.y + r.height / 2,
                                                     length * 0.15, r)
            center_x = p.x
            center_y = p.y
        }
        if(imageEditModel.addStamp(source, center_x, center_y, length) >= 0){
            stampHistoryModel.touch(stamp_id)
        }
    }
    // 画像ファイルを履歴に追加して貼り付ける
    function addStampFile(url, center_x, center_y) {
        var stamp_id = stampHistoryModel.add(url)
        if(stamp_id.length === 0){
            errorLabel.text = qsTr("Failed to load the image.")
            errorLabel.visible = true
            return
        }
        errorLabel.visible = false
        imageEditDialog.placeStamp(stamp_id, center_x, center_y)
    }

    onToolChanged: imageEditModel.selectedLayer = -1
    onOpened: {
        imageEditDialog.editedImage = ""
        imageEditDialog.tool = ""
        errorLabel.visible = false
        imageEditModel.source = imageEditDialog.embedImage
        // 他のダイアログで更新されている場合があるので読み込み直す
        stampHistoryModel.load()
        altTextArea.text = imageEditDialog.embedAlt
        altTextArea.forceActiveFocus()
    }
    onClosed: {
        imageEditDialog.tool = ""
        imageEditModel.clear()
    }

    ImageEditModel {
        id: imageEditModel
    }
    SystemTool {
        id: systemTool
    }
    StampHistoryModel {
        id: stampHistoryModel
    }
    QD.FileDialog {
        id: stampFileDialog
        title: qsTr("Select an image to paste")
        nameFilters: [qsTr("Image files") + " (*.png *.jpg *.jpeg *.gif)"]
        onAccepted: imageEditDialog.addStampFile(selectedFile.toString())
    }
    SettingsC {
        category: "ImageEditDialog"
        property alias mosaicShape: imageEditDialog.mosaicShape
        property alias mosaicStrength: imageEditDialog.mosaicStrength
        property alias drawType: imageEditDialog.drawType
        property alias drawColor: imageEditDialog.drawColor
        property alias drawLineWidth: imageEditDialog.drawLineWidth
        property alias drawFill: imageEditDialog.drawFill
    }
    QD.ColorDialog {
        id: colorDialog
        title: qsTr("Select color")
        onAccepted: imageEditDialog.setDrawColor(selectedColor)
    }

    Shortcut {
        enabled: imageEditDialog.visible && !altTextArea.activeFocus && imageEditModel.canUndo
        sequences: [StandardKey.Undo]
        onActivated: imageEditModel.undo()
    }
    Shortcut {
        enabled: imageEditDialog.visible && !altTextArea.activeFocus && imageEditModel.canRedo
        sequences: [StandardKey.Redo]
        onActivated: imageEditModel.redo()
    }
    Shortcut {
        enabled: imageEditDialog.visible && !altTextArea.activeFocus && imageEditModel.selectedLayer >= 0
        sequences: [StandardKey.Delete, "Backspace"]
        onActivated: imageEditModel.removeLayer(imageEditModel.selectedLayer)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 5 * AdjustedValues.ratio

        RowLayout {
            Layout.fillWidth: true
            spacing: 0
            IconButton {
                enabled: imageEditModel.editable
                iconSource: "../images/crop.png"
                iconSize: AdjustedValues.i18
                flat: true
                foreground: imageEditDialog.cropMode ? Material.accent : Material.foreground
                onClicked: imageEditDialog.toggleTool("crop")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                ToolTip.text: qsTr("Crop")
            }
            IconButton {
                enabled: imageEditModel.editable
                iconSource: "../images/blur_on.png"
                iconSize: AdjustedValues.i18
                flat: true
                foreground: imageEditDialog.tool === "mosaic" ? Material.accent : Material.foreground
                onClicked: imageEditDialog.toggleTool("mosaic")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                ToolTip.text: qsTr("Mosaic")
            }
            IconButton {
                enabled: imageEditModel.editable
                iconSource: "../images/draw.png"
                iconSize: AdjustedValues.i18
                flat: true
                foreground: imageEditDialog.tool === "draw" ? Material.accent : Material.foreground
                onClicked: imageEditDialog.toggleTool("draw")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                ToolTip.text: qsTr("Draw")
            }
            IconButton {
                enabled: imageEditModel.editable
                iconSource: "../images/approval.png"
                iconSize: AdjustedValues.i18
                flat: true
                foreground: imageEditDialog.tool === "stamp" ? Material.accent : Material.foreground
                onClicked: imageEditDialog.toggleTool("stamp")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                ToolTip.text: qsTr("Stamp")
            }
            IconButton {
                enabled: imageEditModel.editable
                iconSource: "../images/rotate_left.png"
                iconSize: AdjustedValues.i18
                flat: true
                onClicked: imageEditModel.rotateLeft()
                ToolTip.visible: hovered
                ToolTip.delay: 500
                ToolTip.text: qsTr("Rotate left")
            }
            IconButton {
                enabled: imageEditModel.editable
                iconSource: "../images/rotate_right.png"
                iconSize: AdjustedValues.i18
                flat: true
                onClicked: imageEditModel.rotateRight()
                ToolTip.visible: hovered
                ToolTip.delay: 500
                ToolTip.text: qsTr("Rotate right")
            }
            Label {
                Layout.leftMargin: 10 * AdjustedValues.ratio
                visible: imageEditModel.ready && !imageEditModel.editable
                font.pointSize: AdjustedValues.f8
                color: Material.color(Material.Grey)
                text: qsTr("This image format cannot be edited.")
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            IconButton {
                enabled: imageEditModel.canUndo
                iconSource: "../images/undo.png"
                iconSize: AdjustedValues.i18
                flat: true
                onClicked: imageEditModel.undo()
                ToolTip.visible: hovered
                ToolTip.delay: 500
                ToolTip.text: qsTr("Undo")
            }
            IconButton {
                enabled: imageEditModel.canRedo
                iconSource: "../images/redo.png"
                iconSize: AdjustedValues.i18
                flat: true
                onClicked: imageEditModel.redo()
                ToolTip.visible: hovered
                ToolTip.delay: 500
                ToolTip.text: qsTr("Redo")
            }
            IconButton {
                enabled: imageEditModel.modified
                iconSource: "../images/restart_alt.png"
                iconSize: AdjustedValues.i18
                flat: true
                onClicked: imageEditModel.resetAll()
                ToolTip.visible: hovered
                ToolTip.delay: 500
                ToolTip.text: qsTr("Reset all edits")
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10 * AdjustedValues.ratio

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                ImageEditCanvas {
                    id: imageEditCanvas
                    anchors.fill: parent
                    editModel: imageEditModel
                    showWholeImage: imageEditDialog.cropMode
                    margin: imageEditDialog.cropMode ? 12 * AdjustedValues.ratio : 0
                }
                ImageCropOverlay {
                    anchors.fill: parent
                    visible: imageEditDialog.cropMode
                    canvas: imageEditCanvas
                    editModel: imageEditModel
                }
                ImageLayerOverlay {
                    anchors.fill: parent
                    visible: imageEditDialog.tool === "mosaic" || imageEditDialog.tool === "draw" ||
                             imageEditDialog.tool === "stamp"
                    canvas: imageEditCanvas
                    editModel: imageEditModel
                    mode: imageEditDialog.tool === "crop" ? "mosaic" : imageEditDialog.tool
                    newLayerShape: imageEditDialog.mosaicShape
                    newLayerStrength: imageEditDialog.mosaicStrength
                    drawType: imageEditDialog.drawType
                    drawColor: imageEditDialog.drawColor
                    drawLineWidth: imageEditDialog.drawLineWidth
                    drawFill: imageEditDialog.drawFill
                }
                // 画像ファイルのドロップで貼り付け
                DropArea {
                    anchors.fill: parent
                    enabled: imageEditModel.editable
                    onEntered: (drag) => drag.accept(Qt.CopyAction)
                    onDropped: (drop) => {
                        if(!drop.hasUrls || drop.urls.length === 0){
                            return
                        }
                        drop.accept(Qt.CopyAction)
                        imageEditDialog.tool = "stamp"
                        var p = imageEditCanvas.mapToImage(drop.x, drop.y)
                        imageEditDialog.addStampFile(drop.urls[0].toString(), p.x, p.y)
                    }
                }
            }

            // ツールの設定
            ColumnLayout {
                Layout.preferredWidth: 200 * AdjustedValues.ratio
                Layout.maximumWidth: 200 * AdjustedValues.ratio
                Layout.fillHeight: true
                visible: imageEditDialog.tool === "mosaic"
                spacing: 5 * AdjustedValues.ratio

                Label {
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("Mosaic")
                }
                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    font.pointSize: AdjustedValues.f8
                    color: Material.color(Material.Grey)
                    text: qsTr("Drag on the image to add an area. Click an area to select it.")
                }
                Label {
                    Layout.topMargin: 5 * AdjustedValues.ratio
                    font.pointSize: AdjustedValues.f8
                    text: qsTr("Shape")
                }
                RowLayout {
                    spacing: 0
                    IconButton {
                        iconSource: "../images/rectangle.png"
                        iconSize: AdjustedValues.i18
                        flat: true
                        foreground: imageEditDialog.currentMosaicShape() === ImageEditModel.ShapeRectangle ? Material.accent : Material.foreground
                        onClicked: imageEditDialog.setMosaicShape(ImageEditModel.ShapeRectangle)
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        ToolTip.text: qsTr("Rectangle")
                    }
                    IconButton {
                        iconSource: "../images/circle.png"
                        iconSize: AdjustedValues.i18
                        flat: true
                        foreground: imageEditDialog.currentMosaicShape() === ImageEditModel.ShapeEllipse ? Material.accent : Material.foreground
                        onClicked: imageEditDialog.setMosaicShape(ImageEditModel.ShapeEllipse)
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        ToolTip.text: qsTr("Ellipse")
                    }
                }
                Label {
                    Layout.topMargin: 5 * AdjustedValues.ratio
                    font.pointSize: AdjustedValues.f8
                    text: qsTr("Coarseness") + " : " + mosaicStrengthSlider.value
                }
                Slider {
                    id: mosaicStrengthSlider
                    Layout.fillWidth: true
                    from: 1
                    to: 10
                    stepSize: 1
                    snapMode: Slider.SnapAlways
                    value: imageEditModel.selectedLayer >= 0 ? imageEditModel.selectedLayerStrength : imageEditDialog.mosaicStrength
                    onMoved: imageEditDialog.setMosaicStrength(value, pressed)
                    onPressedChanged: {
                        if(!pressed){
                            imageEditModel.commitTransient()
                        }
                    }
                }
                Button {
                    Layout.topMargin: 5 * AdjustedValues.ratio
                    enabled: imageEditModel.selectedLayer >= 0
                    flat: true
                    font.pointSize: AdjustedValues.f10
                    icon.source: "../images/delete.png"
                    text: qsTr("Delete area")
                    onClicked: imageEditModel.removeLayer(imageEditModel.selectedLayer)
                }
                Item {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 1
                }
            }

            // 描画の設定
            ColumnLayout {
                Layout.preferredWidth: 200 * AdjustedValues.ratio
                Layout.maximumWidth: 200 * AdjustedValues.ratio
                Layout.fillHeight: true
                visible: imageEditDialog.tool === "draw"
                spacing: 5 * AdjustedValues.ratio

                Label {
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("Draw")
                }
                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    font.pointSize: AdjustedValues.f8
                    color: Material.color(Material.Grey)
                    text: qsTr("Drag on the image to draw. Click a drawing to select it.")
                }
                Label {
                    Layout.topMargin: 5 * AdjustedValues.ratio
                    font.pointSize: AdjustedValues.f8
                    text: qsTr("Tool")
                }
                RowLayout {
                    spacing: 0
                    Repeater {
                        model: [
                            { type: ImageEditModel.TypeLine, icon: "../images/pen_size_2.png", name: qsTr("Line") },
                            { type: ImageEditModel.TypeArrow, icon: "../images/north_east.png", name: qsTr("Arrow") },
                            { type: ImageEditModel.TypeRectangle, icon: "../images/rectangle.png", name: qsTr("Rectangle") },
                            { type: ImageEditModel.TypeEllipse, icon: "../images/circle.png", name: qsTr("Ellipse") },
                            { type: ImageEditModel.TypeFreehand, icon: "../images/gesture.png", name: qsTr("Freehand") }
                        ]
                        IconButton {
                            required property var modelData
                            iconSource: modelData.icon
                            iconSize: AdjustedValues.i18
                            flat: true
                            foreground: imageEditDialog.currentDrawType === modelData.type ? Material.accent : Material.foreground
                            onClicked: imageEditDialog.setDrawType(modelData.type)
                            ToolTip.visible: hovered
                            ToolTip.delay: 500
                            ToolTip.text: modelData.name
                        }
                    }
                }
                Label {
                    Layout.topMargin: 5 * AdjustedValues.ratio
                    font.pointSize: AdjustedValues.f8
                    text: qsTr("Color")
                }
                GridLayout {
                    columns: 5
                    rowSpacing: 4 * AdjustedValues.ratio
                    columnSpacing: 4 * AdjustedValues.ratio
                    Repeater {
                        model: ["#ffffff", "#000000", "#f44336", "#ff9800", "#ffeb3b",
                            "#4caf50", "#00bcd4", "#2196f3", "#9c27b0", "#e91e63"]
                        Rectangle {
                            required property string modelData
                            Layout.preferredWidth: 26 * AdjustedValues.ratio
                            Layout.preferredHeight: 26 * AdjustedValues.ratio
                            radius: width / 2
                            color: modelData
                            border.width: Qt.colorEqual(imageEditDialog.currentDrawColor, modelData) ? 3 : 1
                            border.color: Qt.colorEqual(imageEditDialog.currentDrawColor, modelData) ? Material.accent : Material.color(Material.Grey)
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: imageEditDialog.setDrawColor(parent.modelData)
                            }
                        }
                    }
                }
                RowLayout {
                    Rectangle {
                        Layout.preferredWidth: 26 * AdjustedValues.ratio
                        Layout.preferredHeight: 26 * AdjustedValues.ratio
                        radius: width / 2
                        color: imageEditDialog.currentDrawColor
                        border.width: 1
                        border.color: Material.color(Material.Grey)
                    }
                    Button {
                        flat: true
                        font.pointSize: AdjustedValues.f8
                        icon.source: "../images/palette.png"
                        text: qsTr("Other colors")
                        onClicked: {
                            colorDialog.selectedColor = imageEditDialog.currentDrawColor
                            colorDialog.open()
                        }
                    }
                }
                Label {
                    Layout.topMargin: 5 * AdjustedValues.ratio
                    font.pointSize: AdjustedValues.f8
                    text: qsTr("Width") + " : " + lineWidthSlider.value
                }
                Slider {
                    id: lineWidthSlider
                    Layout.fillWidth: true
                    from: 1
                    to: 20
                    stepSize: 1
                    snapMode: Slider.SnapAlways
                    value: imageEditDialog.currentDrawLineWidth
                    onMoved: imageEditDialog.setDrawLineWidth(value, pressed)
                    onPressedChanged: {
                        if(!pressed){
                            imageEditModel.commitTransient()
                        }
                    }
                }
                CheckBox {
                    visible: imageEditDialog.currentDrawType === ImageEditModel.TypeRectangle ||
                             imageEditDialog.currentDrawType === ImageEditModel.TypeEllipse
                    font.pointSize: AdjustedValues.f8
                    text: qsTr("Fill")
                    checked: imageEditDialog.currentDrawFill
                    onToggled: imageEditDialog.setDrawFill(checked)
                }
                Button {
                    Layout.topMargin: 5 * AdjustedValues.ratio
                    enabled: imageEditDialog.drawingSelected
                    flat: true
                    font.pointSize: AdjustedValues.f10
                    icon.source: "../images/delete.png"
                    text: qsTr("Delete drawing")
                    onClicked: imageEditModel.removeLayer(imageEditModel.selectedLayer)
                }
                Item {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 1
                }
            }

            // スタンプの設定
            ColumnLayout {
                Layout.preferredWidth: 200 * AdjustedValues.ratio
                Layout.maximumWidth: 200 * AdjustedValues.ratio
                Layout.fillHeight: true
                visible: imageEditDialog.tool === "stamp"
                spacing: 5 * AdjustedValues.ratio

                Label {
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("Stamp")
                }
                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    font.pointSize: AdjustedValues.f8
                    color: Material.color(Material.Grey)
                    text: qsTr("Click an image to paste it. You can also drop an image file onto the picture.")
                }
                Button {
                    flat: true
                    font.pointSize: AdjustedValues.f8
                    icon.source: "../images/add_image.png"
                    text: qsTr("Add image")
                    onClicked: stampFileDialog.open()
                }
                Label {
                    Layout.topMargin: 5 * AdjustedValues.ratio
                    font.pointSize: AdjustedValues.f8
                    text: qsTr("Pinned") + " (" + stampHistoryModel.pinnedCount + "/" + stampHistoryModel.maxPinned + ")"
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: 4 * AdjustedValues.ratio
                    Repeater {
                        model: stampHistoryModel
                        StampThumbnail {
                            visible: pinned
                        }
                    }
                }
                Label {
                    Layout.topMargin: 5 * AdjustedValues.ratio
                    font.pointSize: AdjustedValues.f8
                    text: qsTr("History")
                }
                ScrollView {
                    id: stampHistoryScroll
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 60 * AdjustedValues.ratio
                    clip: true
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    Flow {
                        width: stampHistoryScroll.availableWidth
                        spacing: 4 * AdjustedValues.ratio
                        Repeater {
                            model: stampHistoryModel
                            StampThumbnail {
                                visible: !pinned
                            }
                        }
                    }
                }
                Label {
                    Layout.topMargin: 5 * AdjustedValues.ratio
                    visible: imageEditDialog.stampSelected
                    font.pointSize: AdjustedValues.f8
                    text: qsTr("Opacity") + " : " + stampOpacitySlider.value + "%"
                }
                Slider {
                    id: stampOpacitySlider
                    Layout.fillWidth: true
                    visible: imageEditDialog.stampSelected
                    from: 10
                    to: 100
                    stepSize: 5
                    snapMode: Slider.SnapAlways
                    value: imageEditDialog.stampSelected ? imageEditModel.selectedLayerOpacity : 100
                    onMoved: imageEditModel.setLayerOpacity(imageEditModel.selectedLayer, value, pressed)
                    onPressedChanged: {
                        if(!pressed){
                            imageEditModel.commitTransient()
                        }
                    }
                }
                Button {
                    enabled: imageEditDialog.stampSelected
                    flat: true
                    font.pointSize: AdjustedValues.f10
                    icon.source: "../images/delete.png"
                    text: qsTr("Delete stamp")
                    onClicked: imageEditModel.removeLayer(imageEditModel.selectedLayer)
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: 75 * AdjustedValues.ratio
            TextArea {
                id: altTextArea
                verticalAlignment: TextInput.AlignTop
                wrapMode: TextInput.WordWrap
                selectByMouse: true
                font.pointSize: AdjustedValues.f10
                placeholderText: qsTr("Alt text")
                property int realTextLength: systemTool.countText(text)
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Button {
                flat: true
                font.pointSize: AdjustedValues.f10
                text: qsTr("Cancel")
                onClicked: {
                    imageEditDialog.editedImage = ""
                    imageEditDialog.embedAlt = ""
                    altTextArea.text = ""
                    imageEditDialog.reject()
                }
            }
            Label {
                id: errorLabel
                visible: false
                font.pointSize: AdjustedValues.f8
                color: Material.color(Material.Red)
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            Label {
                Layout.alignment: Qt.AlignVCenter
                font.pointSize: AdjustedValues.f8
                text: 5000 - altTextArea.realTextLength
            }
            Button {
                enabled: imageEditModel.ready && altTextArea.realTextLength <= 5000
                font.pointSize: AdjustedValues.f10
                text: qsTr("Apply")
                onClicked: {
                    var edited = ""
                    if(imageEditModel.modified){
                        edited = imageEditModel.exportImage()
                        if(edited.length === 0){
                            errorLabel.text = qsTr("Failed to save the edited image.")
                            errorLabel.visible = true
                            return
                        }
                    }
                    imageEditDialog.tool = ""
                    imageEditDialog.editedImage = edited
                    imageEditDialog.embedAlt = altTextArea.text
                    altTextArea.text = ""
                    imageEditDialog.accept()
                }
            }
        }
    }

    // 履歴の画像（クリックで貼り付け、ピン止め、履歴から削除）
    component StampThumbnail: Rectangle {
        id: thumbnail
        required property string stampId
        required property string source
        required property bool pinned
        width: 56 * AdjustedValues.ratio
        height: width
        radius: 3
        color: Material.theme === Material.Dark ? "#30ffffff" : "#20000000"
        border.width: 1
        border.color: hoverHandler.hovered ? Material.accent : "transparent"

        HoverHandler {
            id: hoverHandler
        }
        Image {
            anchors.fill: parent
            anchors.margins: 4
            fillMode: Image.PreserveAspectFit
            sourceSize.width: 112
            sourceSize.height: 112
            asynchronous: true
            source: thumbnail.source
        }
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: imageEditDialog.placeStamp(thumbnail.stampId)
        }
        ThumbnailButton {
            anchors.left: parent.left
            anchors.top: parent.top
            visible: thumbnail.pinned || hoverHandler.hovered
            readonly property bool available: thumbnail.pinned || stampHistoryModel.pinnedCount < stampHistoryModel.maxPinned
            dimmed: !available
            iconSource: "../images/pin.png"
            iconColor: thumbnail.pinned ? Material.accent : "white"
            toolTipText: thumbnail.pinned ? qsTr("Unpin") :
                                            (available ? qsTr("Pin") : qsTr("Up to %1 images can be pinned.").arg(stampHistoryModel.maxPinned))
            onClicked: {
                if(available){
                    stampHistoryModel.setPinned(thumbnail.stampId, !thumbnail.pinned)
                }
            }
        }
        ThumbnailButton {
            anchors.right: parent.right
            anchors.top: parent.top
            visible: hoverHandler.hovered
            iconSource: "../images/close.png"
            iconColor: "white"
            toolTipText: qsTr("Remove from history")
            onClicked: stampHistoryModel.remove(thumbnail.stampId)
        }
    }
    component ThumbnailButton: Rectangle {
        id: thumbnailButton
        property alias iconSource: buttonIcon.source
        property color iconColor: "white"
        property string toolTipText: ""
        property bool dimmed: false
        signal clicked()
        width: 20 * AdjustedValues.ratio
        height: width
        radius: width / 2
        color: "#a0000000"
        opacity: dimmed ? 0.5 : 1.0
        Image {
            id: buttonIcon
            anchors.centerIn: parent
            width: parent.width * 0.7
            height: width
            layer.enabled: true
            layer.effect: ColorOverlayC {
                color: thumbnailButton.iconColor
            }
        }
        HoverHandler {
            id: buttonHover
        }
        ToolTip.visible: buttonHover.hovered && toolTipText.length > 0
        ToolTip.delay: 500
        ToolTip.text: toolTipText
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: thumbnailButton.clicked()
        }
    }
}
