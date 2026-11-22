import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.controls.imageeditmodel 1.0
import tech.relog.hagoromo.controls.imageeditcanvas 1.0
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
    // 選択中のツール: "" / "crop" / "mosaic"
    property string tool: ""
    readonly property bool cropMode: tool === "crop"
    // 新しく追加するモザイクの設定
    property int mosaicShape: ImageEditModel.ShapeRectangle
    property int mosaicStrength: 5

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

    onToolChanged: imageEditModel.selectedLayer = -1
    onOpened: {
        imageEditDialog.editedImage = ""
        imageEditDialog.tool = ""
        errorLabel.visible = false
        imageEditModel.source = imageEditDialog.embedImage
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
    SettingsC {
        category: "ImageEditDialog"
        property alias mosaicShape: imageEditDialog.mosaicShape
        property alias mosaicStrength: imageEditDialog.mosaicStrength
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
                    visible: imageEditDialog.tool === "mosaic"
                    canvas: imageEditCanvas
                    editModel: imageEditModel
                    newLayerShape: imageEditDialog.mosaicShape
                    newLayerStrength: imageEditDialog.mosaicStrength
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
                text: qsTr("Failed to save the edited image.")
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
}
