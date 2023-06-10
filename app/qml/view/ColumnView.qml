import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.timelinelistmodel 1.0
import tech.relog.hagoromo.notificationlistmodel 1.0
import tech.relog.hagoromo.columnlistmodel 1.0
import tech.relog.hagoromo.searchpostlistmodel 1.0
import tech.relog.hagoromo.searchprofilelistmodel 1.0

import "../controls"
import "../parts"

ColumnLayout {
    id: columnView
    spacing: 0

    LoggingCategory {
        id: logColumn
        name: "tech.relog.hagoromo.ColumnView"
        defaultLogLevel: LoggingCategory.Warning
    }

    property string columnKey: ""
    property int componentType: 0
    property bool autoLoading: false
    property int loadingInterval: 300000
    property string columnValue: ""

    property string accountUuid: ""
    property string service: ""
    property string did: ""
    property string handle: ""
    property string email: ""
    property string accessJwt: ""
    property string refreshJwt: ""
    property string avatar: ""

    property var rootItem: undefined

    property string hoveredLink: ""

    signal requestReply(string account_uuid,
                        string cid, string uri,
                        string reply_root_cid, string reply_root_uri,
                        string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestQuote(string account_uuid,
                        string cid, string uri,
                        string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestViewImages(int index, string paths)

    signal requestMoveToLeft(string key)
    signal requestMoveToRight(string key)
    signal requestRemove(string key)
    signal requestDisplayOfColumnSetting(string key)

    Component {
        id: timelineComponent
        TimelineView {
            model: TimelineListModel {
                autoLoading: columnView.autoLoading
                loadingInterval: columnView.loadingInterval
            }

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(columnView.accountUuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(columnView.accountUuid, cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }

            onRequestViewImages: (index, paths) => columnView.requestViewImages(index, paths)

            onRequestViewProfile: (did) => {
                                      columnStackView.push(profileComponent, { "userDid": did })
                                  }

            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }
    Component {
        id: listNotificationComponent
        NotificationListView {
            model: NotificationListModel {
                autoLoading: columnView.autoLoading
                loadingInterval: columnView.loadingInterval
            }

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(columnView.accountUuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(columnView.accountUuid, cid, uri, avatar, display_name, handle, indexed_at, text)
            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }
            onRequestViewImages: (index, paths) => columnView.requestViewImages(index, paths)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })

            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }
    Component {
        id: postThreadComponent
        PostThreadView {
            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(columnView.accountUuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(columnView.accountUuid, cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }
            onRequestViewImages: (index, paths) => columnView.requestViewImages(index, paths)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })

            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink

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
            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(columnView.accountUuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(columnView.accountUuid, cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }

            onRequestViewImages: (index, paths) => columnView.requestViewImages(index, paths)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink

            onBack: {
                if(!columnStackView.empty){
                    columnStackView.pop()
                }
            }
        }
    }
    Component {
        id: searchPostsComponent
        TimelineView {
            model: SearchPostListModel {
                autoLoading: columnView.autoLoading
                //loadingInterval: columnView.loadingInterval
                text: columnView.columnValue
                searchService: "https://search.bsky.social"
            }

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(columnView.accountUuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(columnView.accountUuid, cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }

            onRequestViewImages: (index, paths) => columnView.requestViewImages(index, paths)

            onRequestViewProfile: (did) => {
                                      columnStackView.push(profileComponent, { "userDid": did })
                                  }

            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }
    Component {
        id: searchProfilesComponent
        ProfileListView {
            accountDid: columnView.did
            unfollowAndRemove: false
            model: SearchProfileListModel {
                autoLoading: columnView.autoLoading
                text: columnView.columnValue
                searchService: "https://search.bsky.social"
            }
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
        }
    }

    function load(){
        console.log(logColumn, "ColumnLayout:componentType=" + componentType)
        if(componentType === 0){
            columnStackView.push(timelineComponent)
            componentTypeLabel.text = qsTr("Home")
        }else if(componentType === 1){
            columnStackView.push(listNotificationComponent)
            componentTypeLabel.text = qsTr("Notifications")
        }else if(componentType === 2){
            columnStackView.push(searchPostsComponent)
            componentTypeLabel.text = qsTr("Search posts") + " : " + columnValue
        }else if(componentType === 3){
            columnStackView.push(searchProfilesComponent)
            componentTypeLabel.text = qsTr("Search users") + " : " + columnValue
        }else{
            columnStackView.push(timelineComponent)
            componentTypeLabel.text = qsTr("Unknown")
        }
    }

    function reflect(){
        // StackViewに積まれているViewに反映
        for(var i=0; i<columnStackView.depth; i++){
            console.log(logColumn, "Reflect : " + i + ", " + columnView.handle)
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
                id: leftIconButton
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                visible: columnStackView.depth > 1
                flat: true
                iconSource: "../images/arrow_left_double.png"
                onClicked: columnStackView.pop(null)
            }

            AvatarImage {
                id: avatarImage
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
                //                Layout.maximumWidth: columnView.width - leftIconButton.Layout.preferredWidth -
                //                                     avatarImage.Layout.preferredWidth - autoIconImage.Layout.preferredWidth -
                //                                     settingButton.Layout.preferredWidth
                Label {
                    id: componentTypeLabel
                    elide: Text.ElideRight
                }
                Label {
                    text: "@" + columnView.handle + " - " + columnView.service
                    font.pointSize: 8
                    elide: Text.ElideRight
                    color: Material.color(Material.Grey)
                }
            }
            Item {
                Layout.fillWidth: true
                height: 1
            }
            Image {
                id: autoIconImage
                Layout.preferredWidth: 16
                Layout.preferredHeight: 16
                Layout.rightMargin: 3
                Layout.alignment: Qt.AlignVCenter
                source: "../images/auto.png"
                layer.enabled: true
                layer.effect: ColorOverlay {
                    color: columnView.autoLoading ? Material.accentColor : Material.color(Material.Grey)
                }
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
                        icon.source: "../images/arrow_back.png"
                        text: qsTr("Move to left")
                        onClicked: requestMoveToLeft(columnKey)
                    }
                    MenuItem {
                        icon.source: "../images/arrow_forward.png"
                        text: qsTr("Move to right")
                        onClicked: requestMoveToRight(columnKey)
                    }
                    MenuItem {
                        icon.source: "../images/delete.png"
                        text: qsTr("Delete column")
                        onClicked: requestRemove(columnKey)
                    }
                    MenuSeparator {}
                    MenuItem {
                        icon.source: "../images/settings.png"
                        text: qsTr("Settings")
                        onClicked: requestDisplayOfColumnSetting(columnKey)
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
