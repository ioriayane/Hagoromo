import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.notificationlistmodel 1.0
import tech.relog.hagoromo.systemtool 1.0

import "../parts"

ScrollView {
    id: notificationListView

    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    clip: true

    property string hoveredLink: ""
    property real fontSizeRatio: 1.0

    property alias listView: rootListView
    property alias model: rootListView.model

    signal requestReply(string cid, string uri,
                          string reply_root_cid, string reply_root_uri,
                          string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestQuote(string cid, string uri, string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestViewThread(string uri)
    signal requestViewImages(int index, var paths)
    signal requestViewProfile(string did)
    signal requestReportPost(string uri, string cid)

    ListView {
        id: rootListView
        anchors.fill: parent
        anchors.rightMargin: parent.ScrollBar.vertical.width

        SystemTool {
            id: systemTool
        }

        header: ItemDelegate {
            width: rootListView.width
            height: 24
            display: AbstractButton.IconOnly
            icon.source: rootListView.model.running ? "" : "../images/expand_less.png"
            onClicked: rootListView.model.getLatest()

            BusyIndicator {
                anchors.centerIn: parent
                width: 24
                height: 24
                visible: rootListView.model.running
            }
        }
        footer: BusyIndicator {
            width: rootListView.width
            height: 24
            visible: rootListView.model.running && rootListView.model.rowCount() > 0
        }

//        add: Transition {
//            NumberAnimation { properties: "x"; from: rootListView.width; duration: 300 }
//        }

        delegate: NotificationDelegate {
            Layout.preferredWidth: rootListView.width

            onRequestViewProfile: (did) => notificationListView.requestViewProfile(did)

            fontSizeRatio: notificationListView.fontSizeRatio

            reason: model.reason
            postAvatarImage.source: model.avatar
            postAvatarImage.onClicked: requestViewProfile(model.did)
            postAuthor.displayName: model.displayName
            postAuthor.handle: model.handle
            postAuthor.indexedAt: model.indexedAt
            recordText: {
                var text = model.recordText
                if(model.recordTextTranslation.length > 0){
                    text = text + "<br/>---<br/>" + model.recordTextTranslation
                }
                return text
            }

            recordDisplayName: model.recordDisplayName
            recordHandle: model.recordHandle
            recordAvatar: model.recordAvatar
            recordIndexedAt: model.recordIndexedAt
            recordRecordText: model.recordRecordText
            recordImagePreview.embedImages: model.recordImages
            recordImagePreview.onRequestViewImages: (index) => requestViewImages(index, model.recordImagesFull)

            postControls.replyButton.iconText: model.replyCount
            postControls.repostButton.iconText: model.repostCount
            postControls.likeButton.iconText: model.likeCount
            postControls.replyButton.onClicked: requestReply(model.cid, model.uri,
                                                               model.replyRootCid, model.replyRootUri,
                                                               model.avatar, model.displayName, model.handle, model.indexedAt, model.recordText)
            postControls.repostMenuItem.onTriggered: rootListView.model.repost(model.index)
            postControls.quoteMenuItem.onTriggered: requestQuote(model.cid, model.uri,
                                                                   model.avatar, model.displayName, model.handle, model.indexedAt, model.recordText)
            postControls.likeButton.onClicked: rootListView.model.like(model.index)
            postControls.onTriggeredTranslate: rootListView.model.translate(model.cid)
            postControls.isReposted: model.isReposted
            postControls.isLiked: model.isLiked
            postControls.postUri: model.uri
            postControls.handle: model.handle
            postControls.onTriggeredCopyToClipboard: systemTool.copyToClipboard(model.recordTextPlain)
            postControls.onTriggeredRequestReport: notificationListView.requestReportPost(model.uri, model.cid)

            onClicked: {
                if(model.reason === NotificationListModel.ReasonLike ||
                        model.reason === NotificationListModel.ReasonRepost){
                    if(model.recordUri.length > 0)
                        requestViewThread(model.recordUri)
                }else if(model.reason === NotificationListModel.ReasonFollow){
                }else if(model.reason === NotificationListModel.ReasonMention){
                }else if(model.reason === NotificationListModel.ReasonReply ||
                         model.reason === NotificationListModel.ReasonQuote){
                    requestViewThread(model.uri)
                }
            }

            recordFrame.onClicked: {
                if(model.reason === NotificationListModel.ReasonQuote){
                    requestViewThread(model.recordUri)
                }
            }
            onHoveredLinkChanged: notificationListView.hoveredLink = hoveredLink
        }
    }
}
