import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

Item {
    id: tagLabelLayout
    clip: true
    implicitHeight: {
        if(repeater.count === 0 || tagLabelLayout.width === 0){
            return 0
        }
        var item = repeater.itemAt(repeater.count-1)
        return item.y + item.height
    }

    property int columnSpacing: 5
    property int rowSpacing: 5
    property int tagSpacing: 0
    property color tagColor: Material.color(Material.BlueGrey,
                                            Material.theme === Material.Light ? Material.Shade100 : Material.Shade800)
    property int tagBorderWidth: 0
    property color tagBorderColor: Material.frameColor
    property alias model: repeater.model
    property alias count: repeater.count
    property string iconSource: "../images/label.png"
    property var iconSources: []
    property real fontPointSize: AdjustedValues.f10

    Repeater {
        id: repeater
        TagLabel {
            color: tagColor
            border.width: tagBorderWidth
            border.color: tagBorderColor
            anchors.left: {
                if(lineStatus === 0){
                    return repeater.parent.left
                }else if(lineStatus === 1){
                    return repeater.parent.left
                }else{
                    return repeater.itemAt(model.index-1).right
                }
            }
            anchors.top: {
                if(lineStatus === 0){
                    return repeater.parent.top
                }else if(lineStatus === 1){
                    return repeater.itemAt(model.index-1).bottom
                }else{
                    return repeater.itemAt(model.index-1).top
                }
            }
            property int lineStatus: {
                if(model.index === 0){
                    return 0
                }else {
                    var item = repeater.itemAt(model.index-1)
                    if((item.x + item.width + 5 + width) > repeater.parent.width){
                        return 1    // 次の行へ折り返す
                    }else{
                        return 2    // 前のアイテムの右隣
                    }
                }
            }
            anchors.topMargin: {
                if(lineStatus === 0){
                    return 0
                }else if(lineStatus === 1){
                    return repeater.parent.rowSpacing
                }else{
                    return 0
                }
            }
            anchors.leftMargin: lineStatus !== 2 ? 0 : repeater.parent.columnSpacing
            text: model.modelData
            source: {
                if(repeater.parent.iconSources.length === repeater.count){
                    var path = repeater.parent.iconSources[model.index]
                    if(!path){
                        enableLayer = true
                        return repeater.parent.iconSource
                    }else{
                        enableLayer = false
                        return path
                    }
                }else{
                    enableLayer = true
                    return repeater.parent.iconSource
                }
            }
            spacing: repeater.parent.tagSpacing
            fontPointSize: tagLabelLayout.fontPointSize
        }
    }
}
