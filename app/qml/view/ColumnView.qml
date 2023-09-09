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
import tech.relog.hagoromo.customfeedlistmodel 1.0
import tech.relog.hagoromo.authorfeedlistmodel 1.0
import tech.relog.hagoromo.anyprofilelistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"

ColumnLayout {
    id: columnView
    spacing: 0

    property string columnKey: ""
    property int componentType: 0

    property alias settings: settings
    property alias account: account
    property string hoveredLink: ""

    signal requestReply(string account_uuid,
                        string cid, string uri,
                        string reply_root_cid, string reply_root_uri,
                        string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestQuote(string account_uuid,
                        string cid, string uri,
                        string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestMention(string account_uuid, string handle)
    signal requestViewAuthorFeed(string account_uuid, string did, string handle)
    signal requestViewImages(int index, var paths, var alts)
    signal requestViewFeedGenerator(string account_uuid, string name, string uri)
    signal requestReportPost(string account_uuid, string uri, string cid)
    signal requestReportAccount(string account_uuid, string did)

    signal requestMoveToLeft(string key)
    signal requestMoveToRight(string key)
    signal requestRemove(string key)
    signal requestDisplayOfColumnSetting(string key)

    ColumnSettings {
        id: settings
    }
    Account {
        id: account
    }

    Component {
        id: timelineComponent
        TimelineView {
            model: TimelineListModel {
                autoLoading: settings.autoLoading
                loadingInterval: settings.loadingInterval

                onErrorOccured: (message) => {console.log(message)}
            }
            accountDid: account.did

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(columnView.account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(columnView.account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }

            onRequestViewImages: (index, paths, alts) => columnView.requestViewImages(index, paths, alts)

            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewFeedGenerator: (name, uri) => columnView.requestViewFeedGenerator(account.uuid, name, uri)
            onRequestViewLikedBy: (uri) => columnStackView.push(likesProfilesComponent, { "targetUri": uri })
            onRequestViewRepostedBy: (uri) => columnStackView.push(repostsProfilesComponent, { "targetUri": uri })
            onRequestReportPost: (uri, cid) => columnView.requestReportPost(account.uuid, uri, cid)

            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }
    Component {
        id: listNotificationComponent
        NotificationListView {
            model: NotificationListModel {
                autoLoading: settings.autoLoading
                loadingInterval: settings.loadingInterval
                visibleLike: settings.visibleLike
                visibleRepost: settings.visibleRepost
                visibleFollow: settings.visibleFollow
                visibleMention: settings.visibleMention
                visibleReply: settings.visibleReply
                visibleQuote: settings.visibleQuote

                onErrorOccured: (message) => {console.log(message)}
            }

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)
            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }
            onRequestViewImages: (index, paths, alts) => columnView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewLikedBy: (uri) => columnStackView.push(likesProfilesComponent, { "targetUri": uri })
            onRequestViewRepostedBy: (uri) => columnStackView.push(repostsProfilesComponent, { "targetUri": uri })
            onRequestReportPost: (uri, cid) => columnView.requestReportPost(account.uuid, uri, cid)

            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }
    Component {
        id: postThreadComponent
        PostThreadView {
            accountDid: account.did

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }
            onRequestViewImages: (index, paths, alts) => columnView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewFeedGenerator: (name, uri) => columnView.requestViewFeedGenerator(account.uuid, name, uri)
            onRequestViewLikedBy: (uri) => columnStackView.push(likesProfilesComponent, { "targetUri": uri })
            onRequestViewRepostedBy: (uri) => columnStackView.push(repostsProfilesComponent, { "targetUri": uri })
            onRequestReportPost: (uri, cid) => columnView.requestReportPost(account.uuid, uri, cid)

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
                            columnView.requestReply(account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)
            onRequestMention: (handle) =>
                              columnView.requestMention(account.uuid, handle)
            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }
            onRequestViewAuthorFeed: (did, handle) =>
                                     columnView.requestViewAuthorFeed(account.uuid, did, handle)

            onRequestViewImages: (index, paths, alts) => columnView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewFeedGenerator: (name, uri) => columnView.requestViewFeedGenerator(account.uuid, name, uri)
            onRequestReportPost: (uri, cid) => columnView.requestReportPost(account.uuid, uri, cid)
            onRequestReportAccount: (did) => columnView.requestReportAccount(account.uuid, did)
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
                autoLoading: settings.autoLoading
                //loadingInterval: settings.loadingInterval
                text: settings.columnValue
                searchService: "https://search.bsky.social"

                onErrorOccured: (message) => {console.log(message)}
            }
            accountDid: account.did

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }

            onRequestViewImages: (index, paths, alts) => columnView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewFeedGenerator: (name, uri) => columnView.requestViewFeedGenerator(account.uuid, name, uri)
            onRequestViewLikedBy: (uri) => columnStackView.push(likesProfilesComponent, { "targetUri": uri })
            onRequestViewRepostedBy: (uri) => columnStackView.push(repostsProfilesComponent, { "targetUri": uri })
            onRequestReportPost: (uri, cid) => columnView.requestReportPost(account.uuid, uri, cid)

            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }
    Component {
        id: searchProfilesComponent
        ProfileListView {
            accountDid: account.did
            unfollowAndRemove: false
            model: SearchProfileListModel {
                autoLoading: settings.autoLoading
                text: settings.columnValue
                searchService: "https://search.bsky.social"

                onErrorOccured: (message) => {console.log(message)}
            }
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }
    Component {
        id: customComponent
        TimelineView {
            model: CustomFeedListModel {
                autoLoading: settings.autoLoading
                loadingInterval: settings.loadingInterval
                uri: settings.columnValue

                onErrorOccured: (message) => {console.log(message)}
            }
            accountDid: account.did

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }

            onRequestViewImages: (index, paths, alts) => columnView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewFeedGenerator: (name, uri) => columnView.requestViewFeedGenerator(account.uuid, name, uri)
            onRequestViewLikedBy: (uri) => columnStackView.push(likesProfilesComponent, { "targetUri": uri })
            onRequestViewRepostedBy: (uri) => columnStackView.push(repostsProfilesComponent, { "targetUri": uri })
            onRequestReportPost: (uri, cid) => columnView.requestReportPost(account.uuid, uri, cid)

            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }

    Component {
        id: authorFeedComponent
        TimelineView {
            model: AuthorFeedListModel {
                autoLoading: settings.autoLoading
                loadingInterval: settings.loadingInterval
                authorDid: settings.columnValue

                onErrorOccured: (message) => {console.log(message)}
            }
            accountDid: account.did

            onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestReply(account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
            onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                            columnView.requestQuote(account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)

            onRequestViewThread: (uri) => {
                                     // スレッドを表示する基準PostのURIはpush()の引数のJSONで設定する
                                     // これはPostThreadViewのプロパティにダイレクトに設定する
                                     columnStackView.push(postThreadComponent, { "postThreadUri": uri })
                                 }

            onRequestViewImages: (index, paths, alts) => columnView.requestViewImages(index, paths, alts)
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onRequestViewFeedGenerator: (name, uri) => columnView.requestViewFeedGenerator(account.uuid, name, uri)
            onRequestViewLikedBy: (uri) => columnStackView.push(likesProfilesComponent, { "targetUri": uri })
            onRequestViewRepostedBy: (uri) => columnStackView.push(repostsProfilesComponent, { "targetUri": uri })
            onRequestReportPost: (uri, cid) => columnView.requestReportPost(account.uuid, uri, cid)

            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }
    Component {
        id: likesProfilesComponent
        AnyProfileListView {
            accountDid: account.did
            unfollowAndRemove: false
            autoLoading: settings.autoLoading
            type: AnyProfileListModel.Like

            onErrorOccured: (message) => { console.log(message) }
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
        }
    }
    Component {
        id: repostsProfilesComponent
        AnyProfileListView {
            accountDid: account.did
            unfollowAndRemove: false
            autoLoading: settings.autoLoading
            type: AnyProfileListModel.Repost

            onErrorOccured: (message) => { console.log(message) }
            onRequestViewProfile: (did) => columnStackView.push(profileComponent, { "userDid": did })
            onHoveredLinkChanged: columnView.hoveredLink = hoveredLink
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
        }else if(componentType === 2){
            columnStackView.push(searchPostsComponent)
            componentTypeLabel.text = qsTr("Search posts") + " : " + settings.columnValue
        }else if(componentType === 3){
            columnStackView.push(searchProfilesComponent)
            componentTypeLabel.text = qsTr("Search users") + " : " + settings.columnValue
        }else if(componentType === 4){
            columnStackView.push(customComponent)
            componentTypeLabel.text = qsTr("Feed") + " : " + settings.columnName
        }else if(componentType === 5){
            columnStackView.push(authorFeedComponent)
            componentTypeLabel.text = qsTr("User") + " : " + settings.columnName
        }else{
            columnStackView.push(timelineComponent)
            componentTypeLabel.text = qsTr("Unknown")
        }
    }

    function reflect(){
        // StackViewに積まれているViewに反映
        for(var i=0; i<columnStackView.depth; i++){
            console.log("Reflect : " + i + ", " + account.handle)
            var item = columnStackView.get(i)
            item.model.setAccount(account.service,
                                  account.did,
                                  account.handle,
                                  account.email,
                                  account.accessJwt,
                                  account.refreshJwt)
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
            id: headerLayout
            anchors.fill: parent
            spacing: 0
            IconButton {
                id: leftIconButton
                Layout.preferredWidth: AdjustedValues.b30
                Layout.preferredHeight: AdjustedValues.b30
                visible: columnStackView.depth > 1
                flat: true
                iconSource: "../images/arrow_left_double.png"
                onClicked: columnStackView.pop(null)
            }

            AvatarImage {
                id: avatarImage
                Layout.leftMargin: 10
                Layout.preferredWidth: AdjustedValues.i24
                Layout.preferredHeight: AdjustedValues.i24
                source: account.avatar
                onClicked: columnStackView.push(profileComponent, { "userDid": account.did })
            }

            ColumnLayout {
                spacing: 0
                Layout.leftMargin: 5
                Layout.topMargin: 5
                Layout.bottomMargin: 5
                Layout.preferredWidth: accountInfoLabel.contentWidth
                Layout.maximumWidth: columnView.width - profileFrame.rightPadding -
                                     leftIconButton.Layout.preferredWidth -
                                     avatarImage.Layout.preferredWidth - avatarImage.Layout.leftMargin -
                                     autoIconImage.Layout.preferredWidth - autoIconImage.Layout.rightMargin -
                                     settingButton.Layout.preferredWidth - 5
                Label {
                    id: componentTypeLabel
                    elide: Text.ElideRight
                    font.pointSize: AdjustedValues.f10
                }
                Label {
                    id: accountInfoLabel
                    text: "@" + account.handle + " - " + account.service
                    font.pointSize: AdjustedValues.f8
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
                Layout.preferredWidth: AdjustedValues.i16
                Layout.preferredHeight: AdjustedValues.i16
                Layout.rightMargin: 3
                Layout.alignment: Qt.AlignVCenter
                source: "../images/auto.png"
                layer.enabled: true
                layer.effect: ColorOverlay {
                    color: settings.autoLoading ? Material.accentColor : Material.color(Material.Grey)
                }
            }
            IconButton {
                id: settingButton
                Layout.preferredWidth: AdjustedValues.b30
                Layout.preferredHeight: AdjustedValues.b24
                iconSource: "../images/settings.png"
                iconSize: AdjustedValues.i16
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
            currentItem.model.setAccount(account.service,
                                         account.did,
                                         account.handle,
                                         account.email,
                                         account.accessJwt,
                                         account.refreshJwt)
            currentItem.model.getLatest()
        }
    }
}
