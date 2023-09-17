import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

Dialog {
    id: messageDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5

    bottomPadding: AdjustedValues.s5

    property int parentWidth: parent.width
    property alias text: messageTextArea.text

    property string status: "normal"

    function show(status, title, message){
        messageDialog.status = status
        titleLabel.text = title
        messageTextArea.text = message
        messageDialog.open()
    }

    ColumnLayout {
        spacing: AdjustedValues.s5

        Label {
            id: titleLabel
            Layout.topMargin: 15
            font.pointSize: AdjustedValues.f10
            text: "Title"
        }
        Frame {
            Layout.topMargin: 5
            background: Rectangle {
                id: backgroundRect
                states: [
                    State {
                        when: messageDialog.status === "error"
                        PropertyChanges {
                            target: backgroundRect
                            border.color: Material.color(Material.Red, Material.Shade600)
                        }
                    }
                ]
                border.width: 1
                border.color: Material.color(Material.Grey, Material.Shade600)
                color: "transparent"
                radius: 2
            }
            contentWidth: messageTextArea.width
            TextArea {
                id: messageTextArea
                width: parentWidth * 0.6 < 800 ? parentWidth * 0.6 : 800
                height: implicitHeight * AdjustedValues.ratio
                wrapMode: TextInput.WordWrap
                selectByMouse: true
                readOnly: true
                background: Item {}
                font.pointSize: AdjustedValues.f10
            }
        }
        Button {
            Layout.alignment: Qt.AlignHCenter
            font.pointSize: AdjustedValues.f10
            flat: true
            text: qsTr("OK")
            onClicked: messageDialog.close()
        }
    }
}
