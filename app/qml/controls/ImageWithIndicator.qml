import QtQuick 2.15
import QtQuick.Controls 2.15

Image {
    id: imageWithIndicator
    BusyIndicator {
        anchors.fill: parent
        visible: imageWithIndicator.status !== Image.Ready
    }
}
