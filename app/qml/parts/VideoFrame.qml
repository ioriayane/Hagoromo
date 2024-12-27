import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../compat"

ClickableFrame {
    id: videoFrame
    contentWidth: thumbImage.width
    contentHeight: thumbImage.height

    topInset: 0
    leftInset: 0
    rightInset: 0
    bottomInset: 0
    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    property string thumbImageSource: ""
    onThumbImageSourceChanged: {
        if(thumbImage.status != Image.Ready){
            thumbImage.source = thumbImageSource
        }
    }

    ImageWithIndicator {
        id: thumbImage
        width: videoFrame.width
        height: {
            if((thumbImage.source + "") !== "") {
                if(sourceSize.width > sourceSize.height){
                    return (width / sourceSize.width) * sourceSize.height
                }else{
                    return width * 9 / 16
                }
            }else{
                return 0
            }
        }
        fillMode: Image.PreserveAspectCrop

        Image {
            anchors.centerIn: parent
            width: AdjustedValues.i60
            height: AdjustedValues.i60
            source: "../images/play_large.png"
        }
    }
}
