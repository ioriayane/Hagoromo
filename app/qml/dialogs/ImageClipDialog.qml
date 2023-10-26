import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import "../parts"

Dialog {
    id: imageClipDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    closePolicy: Popup.NoAutoClose

    property string embedImage: ""

    property int selectedX: -1
    property int selectedY: -1
    property int selectedWidth: 0
    property int selectedHeight: 0

    onEmbedImageChanged: cursorRect.updateDefaultRect()
    onOpened: cursorRect.updateDefaultRect()

    ColumnLayout {
        Image {
            id: image
            Layout.preferredWidth: 300 //* AdjustedValues.ratio
            Layout.preferredHeight: 300 //* AdjustedValues.ratio
            fillMode: Image.PreserveAspectFit
            source: imageClipDialog.embedImage

            property real offsetX: (image.width - image.paintedWidth) / 2
            property real offsetY: (image.height - image.paintedHeight) / 2
            property real widthRatio: image.sourceSize.width / image.paintedWidth
            property real heightRatio: image.sourceSize.height / image.paintedHeight

            CursorRect {
                id: cursorRect
                anchors.fill: parent
                function updateDefaultRect(){
                    cursorX = image.offsetX
                    cursorY = image.offsetY
                    cursorWidth = image.paintedWidth
                    cursorHeight = image.paintedHeight
                }
            }
        }

        RowLayout {
            Button {
                flat: true
                font.pointSize: 10 //AdjustedValues.f10
                text: qsTr("Cancel")
                onClicked: {
                    selectedX = -1
                    selectedY = -1
                    selectedWidth = 0
                    selectedHeight = 0
                    imageClipDialog.embedImage = ""
                    imageClipDialog.reject()
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            Button {
                font.pointSize: 10 //AdjustedValues.f10
                text: qsTr("Apply")
                onClicked: {
                    selectedX = (cursorRect.cursorX - image.offsetX) * image.widthRatio
                    selectedY = (cursorRect.cursorY - image.offsetY) * image.heightRatio
                    selectedWidth = cursorRect.cursorWidth * image.widthRatio
                    selectedHeight = cursorRect.cursorHeight * image.heightRatio

                    imageClipDialog.accept()
                }
            }
        }
    }
}
