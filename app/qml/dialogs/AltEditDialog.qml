import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

Dialog {
    id: altEditDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    closePolicy: Popup.NoAutoClose

    property string embedImage: ""
    property string embedAlt: ""

    onOpened: {
        altTextArea.text = altEditDialog.embedAlt
        altTextArea.forceActiveFocus()
    }

    SystemTool {
        id: systemTool
    }

    ColumnLayout {
        Image {
            id: image
            Layout.preferredWidth: 300 * AdjustedValues.ratio
            Layout.preferredHeight: image.paintedWidth > image.paintedHeight ? image.paintedHeight : 300 * AdjustedValues.ratio
            Layout.maximumWidth: 300 * AdjustedValues.ratio
            Layout.maximumHeight: 300 * AdjustedValues.ratio
            fillMode: Image.PreserveAspectFit
            source: altEditDialog.embedImage
        }
        ScrollView {
            Layout.preferredWidth: 300 * AdjustedValues.ratio
            Layout.preferredHeight: 75 * AdjustedValues.ratio
            TextArea {
                id: altTextArea
                verticalAlignment: TextInput.AlignTop
                wrapMode: TextInput.WordWrap
                selectByMouse: true
                font.pointSize: AdjustedValues.f10
                property int realTextLength: systemTool.countText(text)
            }
        }
        RowLayout {
            Button {
                flat: true
                font.pointSize: AdjustedValues.f10
                text: qsTr("Cancel")
                onClicked: {
                    altEditDialog.embedImage = ""
                    altEditDialog.embedAlt = ""
                    altTextArea.text = ""
                    altEditDialog.close()
                }
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
                enabled: altTextArea.realTextLength > 0 &&
                         altTextArea.realTextLength <= 5000
                font.pointSize: AdjustedValues.f10
                text: qsTr("Add")
                onClicked: {
                    altEditDialog.embedImage = ""
                    altEditDialog.embedAlt = altTextArea.text
                    altTextArea.text = ""
                    altEditDialog.accept()
                }
            }
        }
    }
}
