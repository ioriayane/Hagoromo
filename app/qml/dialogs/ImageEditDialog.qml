import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.controls.imageeditmodel 1.0
import tech.relog.hagoromo.controls.imageeditcanvas 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"

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
    property bool cropMode: false

    onOpened: {
        imageEditDialog.editedImage = ""
        imageEditDialog.cropMode = false
        errorLabel.visible = false
        imageEditModel.source = imageEditDialog.embedImage
        altTextArea.text = imageEditDialog.embedAlt
        altTextArea.forceActiveFocus()
    }
    onClosed: {
        imageEditDialog.cropMode = false
        imageEditModel.clear()
    }

    ImageEditModel {
        id: imageEditModel
    }
    SystemTool {
        id: systemTool
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
                onClicked: imageEditDialog.cropMode = !imageEditDialog.cropMode
                ToolTip.visible: hovered
                ToolTip.delay: 500
                ToolTip.text: qsTr("Crop")
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
                    imageEditDialog.cropMode = false
                    imageEditDialog.editedImage = edited
                    imageEditDialog.embedAlt = altTextArea.text
                    altTextArea.text = ""
                    imageEditDialog.accept()
                }
            }
        }
    }
}
