import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../compat"

ClickableFrame {
    id: videoFrame
    contentWidth: thumbImage.paintedWidth
    contentHeight: thumbImage.paintedHeight

    topInset: 0
    leftInset: 0
    rightInset: 0
    bottomInset: 0
    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    property alias thumbImage: thumbImage

    ImageWithIndicator {
        id: thumbImage
        width: videoFrame.width
        height: status !== Image.Null ? (width / sourceSize.width) * sourceSize.height : 5
        fillMode: Image.PreserveAspectCrop

        Image {
            anchors.centerIn: parent
            width: AdjustedValues.i50
            height: AdjustedValues.i50
            layer.enabled: true
            layer.effect: ColorOverlayC {
                color: "white"
            }
            source: "../images/play_large.png"
        }
    }
}
