import QtQuick 2.15
import QtQuick.Controls 2.15

Pane {
    Label {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 10
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        wrapMode: Text.WordWrap
        text: qsTr("You are not authorized to use chat.\nPlease recreate AppPassword in the official application.")
    }
}
