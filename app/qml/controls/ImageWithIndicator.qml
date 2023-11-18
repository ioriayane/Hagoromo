import QtQuick 2.15
import QtQuick.Controls 2.15

AnimatedImage {
    id: imageWithIndicator
    onStatusChanged: playing = (status == AnimatedImage.Ready)

    BusyIndicator {
        anchors.centerIn: parent
        width: parent.width > 100 ? 100 : parent.width
        height: parent.height > 100 ? 100 : parent.height

        visible: imageWithIndicator.status === Image.Loading ||
                 imageWithIndicator.status === Image.Error
    }
}
