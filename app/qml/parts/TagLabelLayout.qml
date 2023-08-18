import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    clip: true
    height: {
        if(repeater.count === 0){
            return 0
        }
        var item = repeater.itemAt(repeater.count-1)
        return item.y + item.height
    }

    property int columnSpacing: 5
    property int rowSpacing: 5
    property alias model: repeater.model
    property alias count: repeater.count

    Repeater {
        id: repeater
        TagLabel {
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
        }
    }
}
