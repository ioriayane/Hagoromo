import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.notificationlistmodel 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "../parts"

ScrollView {
    id: notificationListView

    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    clip: true

    property string hoveredLink: ""
    property int imageLayoutType: 1

    property alias listView: rootListView
    property alias model: rootListView.model

    signal requestReply(string cid, string uri,
                          string reply_root_cid, string reply_root_uri,
                          string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestQuote(string cid, string uri, string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestViewThread(string uri)
    signal requestViewImages(int index, var paths, var alts)
    signal requestViewProfile(string did)
    signal requestViewFeedGenerator(string name, string uri)
    signal requestViewListFeed(string uri, string name)
    signal requestViewLikedBy(string uri)
    signal requestViewRepostedBy(string uri)
    signal requestViewSearchPosts(string text)
    signal requestReportPost(string uri, string cid)
    signal requestAddMutedWord(string text)

    ListView {
        id: rootListView
        anchors.fill: parent
        anchors.rightMargin: parent.ScrollBar.vertical.width
        maximumFlickVelocity: AdjustedValues.maximumFlickVelocity

        SystemTool {
            id: systemTool
        }

        onMovementEnded: {
            if(atYEnd){
                rootListView.model.getNext()
            }
        }

        header: ItemDelegate {
            width: rootListView.width
            height: AdjustedValues.h24
            display: AbstractButton.IconOnly
            icon.source: rootListView.model.running ? "" : "../images/expand_less.png"
            onClicked: rootListView.model.getLatest()

            BusyIndicator {
                anchors.centerIn: parent
                width: AdjustedValues.i24
                height: AdjustedValues.i24
                visible: rootListView.model.running
            }
        }
        footer: BusyIndicator {
            width: rootListView.width
            height: AdjustedValues.i24
            visible: rootListView.model.running && rootListView.model.rowCount() > 0
        }

//        add: Transition {
//            NumberAnimation { properties: "x"; from: rootListView.width; duration: 300 }
//        }

        delegate: NotificationDelegate {
            Layout.preferredWidth: rootListView.width

            onRequestViewProfile: (did) => notificationListView.requestViewProfile(did)
            onRequestViewSearchPosts: (text) => notificationListView.requestViewSearchPosts(text)
            onRequestAddMutedWord: (text) => notificationListView.requestAddMutedWord(text)

            moderationFrame.visible: model.muted
            userFilterMatched: model.userFilterMatched
            userFilterMessage: model.userFilterMessage

            reason: model.reason
            postAvatarImage.source: model.avatar
            postAvatarImage.onClicked: requestViewProfile(model.did)
            aggregatedAvatarImages.avatars: model.aggregatedAvatars
            aggregatedAvatarImages.displayNames: model.aggregatedDisplayNames
            aggregatedAvatarImages.dids: model.aggregatedDids
            aggregatedAvatarImages.handles: model.aggregatedHandles
            aggregatedAvatarImages.indexedAts: model.aggregatedIndexedAts
            aggregatedAvatarImages.onSelectAvatar: (did) => requestViewProfile(did)
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
            contentFilterFrame.visible: model.contentFilterMatched
            contentFilterFrame.labelText: model.contentFilterMessage
            contentMediaFilterFrame.visible: model.contentMediaFilterMatched
            contentMediaFilterFrame.labelText: model.contentMediaFilterMessage
            postImagePreview.visible: contentMediaFilterFrame.showContent && model.embedImages.length > 0
            postImagePreview.layoutType: notificationListView.imageLayoutType
            postImagePreview.embedImages: model.embedImages
            postImagePreview.embedAlts: model.embedImagesAlt
            postImagePreview.onRequestViewImages: (index) => requestViewImages(index, model.embedImagesFull, model.embedImagesAlt)

            quoteRecordDisplayName: model.quoteRecordDisplayName
            quoteRecordHandle: model.quoteRecordHandle
            quoteRecordAvatar: model.quoteRecordAvatar
            quoteRecordIndexedAt: model.quoteRecordIndexedAt
            quoteRecordRecordText: model.quoteRecordRecordText
            quoteRecordImagePreview.layoutType: notificationListView.imageLayoutType
            quoteRecordImagePreview.embedImages: model.quoteRecordEmbedImages
            quoteRecordImagePreview.embedAlts: model.quoteRecordEmbedImagesAlt
            quoteRecordImagePreview.onRequestViewImages: (index) => requestViewImages(index, model.quoteRecordEmbedImagesFull, model.quoteRecordEmbedImagesAlt)

            externalLinkFrame.visible: model.hasExternalLink && contentMediaFilterFrame.showContent
            externalLinkFrame.onClicked: Qt.openUrlExternally(model.externalLinkUri)
            externalLinkFrame.thumbImage.source: model.externalLinkThumb
            externalLinkFrame.titleLabel.text: model.externalLinkTitle
            externalLinkFrame.uriLabel.text: model.externalLinkUri
            externalLinkFrame.descriptionLabel.text: model.externalLinkDescription

            feedGeneratorFrame.visible: model.hasFeedGenerator && contentMediaFilterFrame.showContent
            feedGeneratorFrame.onClicked: requestViewFeedGenerator(model.feedGeneratorDisplayName, model.feedGeneratorUri)
            feedGeneratorFrame.avatarImage.source: model.feedGeneratorAvatar
            feedGeneratorFrame.displayNameLabel.text: model.feedGeneratorDisplayName
            feedGeneratorFrame.creatorHandleLabel.text: model.feedGeneratorCreatorHandle
            feedGeneratorFrame.likeCountLabel.text: model.feedGeneratorLikeCount

            listLinkCardFrame.visible: model.hasListLink && contentMediaFilterFrame.showContent
            listLinkCardFrame.onClicked: requestViewListFeed(model.listLinkUri, model.listLinkDisplayName)
            listLinkCardFrame.avatarImage.source: model.listLinkAvatar
            listLinkCardFrame.displayNameLabel.text: model.listLinkDisplayName
            listLinkCardFrame.creatorHandleLabel.text: model.listLinkCreatorHandle
            listLinkCardFrame.descriptionLabel.text: model.listLinkDescription

            postControls.replyButton.iconText: model.replyCount
            postControls.repostButton.iconText: model.repostCount
            postControls.likeButton.iconText: model.likeCount
            postControls.replyButton.enabled: !model.replyDisabled
            postControls.repostButton.enabled: !model.runningRepost
            postControls.likeButton.enabled: !model.runningLike
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
            postControls.onTriggeredCopyPostUrlToClipboard: systemTool.copyToClipboard(rootListView.model.getItemOfficialUrl(model.index))
            postControls.onTriggeredRequestViewLikedBy: notificationListView.requestViewLikedBy(model.uri)
            postControls.onTriggeredRequestViewRepostedBy: notificationListView.requestViewRepostedBy(model.uri)
            postControls.onTriggeredRequestReport: notificationListView.requestReportPost(model.uri, model.cid)

            onClicked: {
                if(model.reason === NotificationListModel.ReasonLike ||
                        model.reason === NotificationListModel.ReasonRepost){
                    if(model.quoteRecordUri.length > 0)
                        requestViewThread(model.quoteRecordUri)
                }else if(model.reason === NotificationListModel.ReasonFollow){
                }else if(model.reason === NotificationListModel.ReasonMention){
                }else if(model.reason === NotificationListModel.ReasonReply ||
                         model.reason === NotificationListModel.ReasonQuote){
                    requestViewThread(model.uri)
                }
            }

            quoteRecordFrame.visible: (model.reason === NotificationListModel.ReasonQuote) &&
                                      model.quoteRecordCid.length > 0 &&
                                      contentMediaFilterFrame.showContent
            quoteRecordFrame.onClicked: {
                if(model.reason === NotificationListModel.ReasonQuote){
                    requestViewThread(model.quoteRecordUri)
                }
            }
            onHoveredLinkChanged: notificationListView.hoveredLink = hoveredLink
        }
    }
}
