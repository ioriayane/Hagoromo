import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.columnlistmodel 1.0

import "../controls"

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

    property var rootItem: undefined

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
            componentTypeLabel.text = qsTr("Home")
        }else if(componentType === 1){
            loader.sourceComponent = listNotificationComponent
            componentTypeLabel.text = qsTr("Notifications")
        }else{
            loader.sourceComponent = timelineComponent
        }
    }

    Frame {
        id: profileFrame
        Layout.fillWidth: true
        Layout.topMargin: 1
        leftPadding: 10
        topPadding: 3
        rightPadding: 10
        bottomPadding: 3

        RowLayout {
            anchors.fill: parent
            ColumnLayout {
                spacing: 0
                Label {
                    id: componentTypeLabel
                }
                Label {
                    text: "@" + columnView.handle
                    font.pointSize: 8
                }
            }
            Item {
                Layout.fillWidth: true
                height: 1
            }
            IconButton {
                id: settingButton
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                iconSource: "../images/settings.png"
                onClicked: {
                    var point = settingButton.mapToItem(rootItem, settingButton.pressX, settingButton.pressY)
                    popup.x = point.x - popup.width
                    popup.y = point.y
                    popup.open()
                }
                Popup {
                    id: popup
                    parent: Overlay.overlay
                    width: popupItemLayout.width
                    height: popupItemLayout.height
                    padding: 0

                    ColumnLayout {
                        id: popupItemLayout
                        spacing: 0
                        ItemDelegate {
                            Layout.fillWidth: true
                            text: qsTr("Move left")
                        }
                        ItemDelegate {
                            Layout.fillWidth: true
                            text: qsTr("Move right")
                        }
                        ItemDelegate {
                            Layout.fillWidth: true
                            icon.source: "../images/delete.png"
                            text: qsTr("Delete column")
                        }
                    }
                }
            }
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
