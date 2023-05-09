import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.columnlistmodel 1.0

ColumnLayout {
    id: columnView
    spacing: 0

    property int componentType: 0

    property string service: ""
    property string did: ""
    property string handle: ""
    property string email: ""
    property string accessJwt: ""
    property string refreshJwt: ""

    Component {
        id: timelineComponent
        TimelineView {
        }
    }
    Component {
        id: listNotificationComponent
        NotificationListView {
        }
    }

    function load(){
        console.log("ColumnLayout:componentType=" + componentType)
        if(componentType === 0){
            loader.sourceComponent = timelineComponent
        }else if(componentType === 1){
            loader.sourceComponent = listNotificationComponent
        }else{
            loader.sourceComponent = timelineComponent
        }
    }

    Frame {
        id: profileFrame
        Layout.fillWidth: true
        Layout.topMargin: 1
        Label {
            text: columnView.handle
        }
    }

    Loader {
        id: loader
        Layout.fillWidth: true
        Layout.fillHeight: true

        onLoaded: {
            item.model.setAccount(columnView.service,
                                  columnView.did,
                                  columnView.handle,
                                  columnView.email,
                                  columnView.accessJwt,
                                  columnView.refreshJwt)
            item.model.getLatest()
        }
    }

}
