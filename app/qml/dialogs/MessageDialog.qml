import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

Dialog {
    id: messageDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
//    standardButtons: DialogButtonBox.Ok

    property int parentWidth: parent.width
    property alias text: messageTextArea.text

    property string status: "normal"

    function show(status, message){
        messageDialog.status = status
        messageTextArea.text = message
        messageDialog.open()
    }

    ColumnLayout {
        Frame {
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
                wrapMode: TextInput.WordWrap
                selectByMouse: true
                readOnly: true
                background: Item {}
            }
        }
        Button {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("OK")
            onClicked: messageDialog.close()
        }
    }
}
