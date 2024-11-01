import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

//ドラッグ状態に反応してことを表す四角
Rectangle {
    id: dropRect
    radius: 4
    color: Material.color(Material.Grey)
    opacity: 0
    states: State {
        //ドラッグ状態で領域内にいたら背景色と文字色を変更
        when: imageDropArea.containsDrag
        PropertyChanges { target: dropRect; opacity: 0.5 }
        PropertyChanges { target: dropRectMessage; opacity: 1 }
    }
    signal dropped(var urls)

    //ドラッグ状態で領域内にいるときの説明
    Text {
        id: dropRectMessage
        anchors.centerIn: parent
        color: "black"
        text: qsTr("Detecting...")
        font.pointSize: AdjustedValues.f14
    }
    //ドロップの受付
    DropArea {
        id: imageDropArea
        anchors.fill: parent
        keys: ["text/uri-list"]
        onDropped: (drop) => {
                       if(drop.hasUrls){
                           var new_urls = []
                           for(var i=0; i<drop.urls.length; i++){
                               new_urls.push(drop.urls[i])
                           }
                           dropRect.dropped(new_urls)
                       }
                   }
    }
}
