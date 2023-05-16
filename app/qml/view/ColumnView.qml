import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.columnlistmodel 1.0
import tech.relog.hagoromo.createrecord 1.0

import "../controls"

ColumnLayout {
    id: columnView
    spacing: 0

    property int componentType: 0

    property string accountUuid: ""
    property string service: ""
    property string did: ""
    property string handle: ""
    property string email: ""
    property string accessJwt: ""
    property string refreshJwt: ""

    property var rootItem: undefined

    signal requestedReply(string account_uuid,
                          string cid, string uri,
                          string reply_root_cid, string reply_root_uri,
                          string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestedQuote(string account_uuid,
                          string cid, string uri,
                          string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestedViewImages(int index, string paths)

    CreateRecord {
        id: createRecord
        onFinished: (success) => console.log("ColumnView::CreateRecord::onFinished:" + success)
    }

    Component {
        id: timelineComponent
        TimelineView {
            onRequestedReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                              columnView.requestedReply(columnView.accountUuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestedRepost: (cid, uri) => createRecord.repost(cid, uri)
            onRequestedQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                              columnView.requestedQuote(columnView.accountUuid, cid, uri, avatar, display_name, handle, indexed_at, text)
            onRequestedLike: (cid, uri) => createRecord.like(cid, uri)

            onRequestedViewThread: (uri) => {
                                       console.log("View Thread : " + uri)
                                       // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                       // これはPostThreadViewのプロパティにダイレクトに設定する
                                       loader.push(postThreadComponent, { "postThreadUri": uri })
                                   }

            onRequestedViewImages: (index, paths) => columnView.requestedViewImages(index, paths)
        }
    }

    Component {
        id: listNotificationComponent
        NotificationListView {
        }
    }
    Component {
        id: postThreadComponent
        PostThreadView {
            onRequestedReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                              columnView.requestedReply(columnView.accountUuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestedRepost: (cid, uri) => createRecord.repost(cid, uri)
            onRequestedQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                              columnView.requestedQuote(columnView.accountUuid, cid, uri, avatar, display_name, handle, indexed_at, text)
            onRequestedLike: (cid, uri) => createRecord.like(cid, uri)

            onRequestedViewImages: (index, paths) => columnView.requestedViewImages(index, paths)

            onBack: {
                if(!loader.empty){
                    loader.pop()
                }
            }
        }
    }

    function load(){
        console.log("ColumnLayout:componentType=" + componentType)
        if(componentType === 0){
            loader.push(timelineComponent)
            componentTypeLabel.text = qsTr("Home")
        }else if(componentType === 1){
            loader.push(listNotificationComponent)
            componentTypeLabel.text = qsTr("Notifications")
        }else{
            loader.push(timelineComponent)
            componentTypeLabel.text = qsTr("Unknown")
        }
        createRecord.setAccount(service, did, handle, email, accessJwt, refreshJwt)
    }

    function reflect(){
        // StackViewに積まれているViewに反映
        for(var i=0; i<loader.depth; i++){
            console.log("Reflect : " + i + ", " + columnView.handle)
            var item = loader.get(i)
            item.model.setAccount(columnView.service,
                                  columnView.did,
                                  columnView.handle,
                                  columnView.email,
                                  columnView.accessJwt,
                                  columnView.refreshJwt)
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
                    text: "@" + columnView.handle + " - " + columnView.service
                    font.pointSize: 8
                    color: Material.color(Material.Grey)
                }
            }
            Item {
                Layout.fillWidth: true
                height: 1
            }
            IconButton {
                id: settingButton
                Layout.preferredWidth: 30
                Layout.preferredHeight: 24
                iconSource: "../images/settings.png"
                iconSize: 16
                onClicked: popup.open()
                Menu {
                    id: popup
                    MenuItem {
                        text: qsTr("Move left")
                    }
                    MenuItem {
                        text: qsTr("Move right")
                    }
                    MenuItem {
                        icon.source: "../images/delete.png"
                        text: qsTr("Delete column")
                    }
                }
            }
        }
    }

    StackView {
        id: loader
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true

        onCurrentItemChanged: {
            if(currentItem.model === undefined)
                return
            currentItem.model.setAccount(columnView.service,
                                         columnView.did,
                                         columnView.handle,
                                         columnView.email,
                                         columnView.accessJwt,
                                         columnView.refreshJwt)
            currentItem.model.getLatest()
        }
    }

}
