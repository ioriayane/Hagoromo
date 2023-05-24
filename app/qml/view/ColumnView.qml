import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.timelinelistmodel 1.0
import tech.relog.hagoromo.columnlistmodel 1.0
import tech.relog.hagoromo.createrecord 1.0

import "../controls"
import "../parts"

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
    property string avatar: ""

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
            model: TimelineListModel {
                autoLoading: true
            }

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
                                       columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                   }

            onRequestedViewImages: (index, paths) => columnView.requestedViewImages(index, paths)

            onRequestedViewProfile: (did) => {
                                        console.log("View profile : " + did)
                                        columnStackView.push(profileComponent, { "userDid": did })
                                    }
        }
    }

    Component {
        id: listNotificationComponent
        NotificationListView {
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
                                       columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                   }
            onRequestedViewImages: (index, paths) => columnView.requestedViewImages(index, paths)
            onRequestedViewProfile: (did) => {
                                        console.log("View profile : " + did)
                                        columnStackView.push(profileComponent, { "userDid": did })
                                    }
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

            onRequestedViewThread: (uri) => {
                                       console.log("View Thread : " + uri)
                                       // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                       // これはPostThreadViewのプロパティにダイレクトに設定する
                                       columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                   }
            onRequestedViewImages: (index, paths) => columnView.requestedViewImages(index, paths)
            onRequestedViewProfile: (did) => {
                                        console.log("View profile : " + did)
                                        columnStackView.push(profileComponent, { "userDid": did })
                                    }

            onBack: {
                if(!columnStackView.empty){
                    columnStackView.pop()
                }
            }
        }
    }
    Component {
        id: profileComponent
        ProfileView {
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
                                       columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                   }

            onRequestedViewImages: (index, paths) => columnView.requestedViewImages(index, paths)

            onRequestedViewProfile: (did) => {
                                        console.log("View profile : " + did)
                                        columnStackView.push(profileComponent, { "userDid": did })
                                    }
            onRequestedFollow: (did) => createRecord.follow(did)
            onBack: {
                if(!columnStackView.empty){
                    columnStackView.pop()
                }
            }
        }
    }

    function load(){
        console.log("ColumnLayout:componentType=" + componentType)
        if(componentType === 0){
            columnStackView.push(timelineComponent)
            componentTypeLabel.text = qsTr("Home")
        }else if(componentType === 1){
            columnStackView.push(listNotificationComponent)
            componentTypeLabel.text = qsTr("Notifications")
        }else{
            columnStackView.push(timelineComponent)
            componentTypeLabel.text = qsTr("Unknown")
        }
        createRecord.setAccount(service, did, handle, email, accessJwt, refreshJwt)
    }

    function reflect(){
        // StackViewに積まれているViewに反映
        for(var i=0; i<columnStackView.depth; i++){
            console.log("Reflect : " + i + ", " + columnView.handle)
            var item = columnStackView.get(i)
            item.model.setAccount(columnView.service,
                                  columnView.did,
                                  columnView.handle,
                                  columnView.email,
                                  columnView.accessJwt,
                                  columnView.refreshJwt)
        }
    }

    ClickableFrame {
        id: profileFrame
        Layout.fillWidth: true
        Layout.topMargin: 1
        leftPadding: 0
        topPadding: 0
        rightPadding: 10
        bottomPadding: 0

        onClicked: (mouse) => {
                       if(columnStackView.currentItem.listView){
                           columnStackView.currentItem.listView.positionViewAtBeginning()
                       }
                   }

        RowLayout {
            anchors.fill: parent
            spacing: 0
            IconButton {
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                visible: columnStackView.depth > 1
                flat: true
                iconSource: "../images/arrow_left_double.png"
                onClicked: columnStackView.pop(null)
            }

            AvatarImage {
                Layout.leftMargin: 10
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                source: columnView.avatar
            }

            ColumnLayout {
                spacing: 0
                Layout.leftMargin: 5
                Layout.topMargin: 5
                Layout.bottomMargin: 5
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
        id: columnStackView
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true

        onCurrentItemChanged: {
            if(currentItem.model === undefined)
                return
            if(currentItem.model.rowCount() > 0)
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
