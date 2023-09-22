import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.postthreadlistmodel 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"

ColumnLayout {
    id: postThreadView

    property string hoveredLink: ""
    property string accountDid: ""   // 取得するユーザー

    property alias postThreadUri: postThreadListModel.postThreadUri
    property alias listView: rootListView
    property alias model: postThreadListModel

    signal requestReply(string cid, string uri,
                        string reply_root_cid, string reply_root_uri,
                        string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestQuote(string cid, string uri, string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestViewThread(string uri)
    signal requestViewImages(int index, var paths, var alts)
    signal requestViewProfile(string did)
    signal requestViewFeedGenerator(string name, string uri)
    signal requestViewLikedBy(string uri)
    signal requestViewRepostedBy(string uri)
    signal requestReportPost(string uri, string cid)

    signal errorOccured(string code, string message)
    signal back()

    Frame {
        Layout.fillWidth: true
        leftPadding: 0
        topPadding: 0
        rightPadding: 10
        bottomPadding: 0

        RowLayout {
            IconButton {
                Layout.preferredWidth: AdjustedValues.b30
                Layout.preferredHeight: AdjustedValues.b30
                flat: true
                iconSource: "../images/arrow_left_single.png"
                onClicked: postThreadView.back()
            }
            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 10
                font.pointSize: AdjustedValues.f10
                text: qsTr("Post thread")
            }
        }
    }

    ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true

        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        clip: true

        ListView {
            id: rootListView
            anchors.fill: parent
            anchors.rightMargin: parent.ScrollBar.vertical.width

            SystemTool {
                id: systemTool
            }

            model: PostThreadListModel {
                id: postThreadListModel
                autoLoading: false

                onErrorOccured: (code, message) => profileView.errorOccured(code, message)
            }

            header: Item {
                width: rootListView.width
                height: AdjustedValues.h24

                BusyIndicator {
                    anchors.centerIn: parent
                    width: AdjustedValues.i24
                    height: AdjustedValues.i24
                    visible: postThreadListModel.running
                }
            }

            //            add: Transition {
            //                NumberAnimation { properties: "x"; from: rootListView.width; duration: 300 }
            //            }

            delegate: PostDelegate {
                Layout.preferredWidth: rootListView.width

                //自分から自分へは移動しない
                onClicked: (mouse) => {
                               if(postThreadUri !== model.uri){
                                   requestViewThread(model.uri)
                               }
                           }
                onRequestViewProfile: (did) => postThreadView.requestViewProfile(did)

                moderationFrame.visible: model.muted
                userFilterMatched: model.userFilterMatched
                userFilterMessage: model.userFilterMessage

                repostReactionAuthor.visible: model.isRepostedBy
                repostReactionAuthor.displayName: model.repostedByDisplayName
                repostReactionAuthor.handle: model.repostedByHandle
                replyReactionAuthor.visible: model.hasReply
                replyReactionAuthor.displayName: model.replyParentDisplayName
                replyReactionAuthor.handle: model.replyParentHandle

                postAvatarImage.source: model.avatar
                postAvatarImage.onClicked: requestViewProfile(model.did)
                postAuthor.displayName: model.displayName
                postAuthor.handle: model.handle
                postAuthor.indexedAt: (postThreadUri === model.uri) ? "" : model.indexedAt
                recordText.text: {
                    var text = model.recordText
                    if(model.recordTextTranslation.length > 0){
                        text = text + "<br/>\n---<br/>\n" + model.recordTextTranslation
                    }
                    return text
                }
                contentFilterFrame.visible: model.contentFilterMatched
                contentFilterFrame.labelText: model.contentFilterMessage
                contentMediaFilterFrame.visible: model.contentMediaFilterMatched
                contentMediaFilterFrame.labelText: model.contentMediaFilterMessage
                postImagePreview.embedImages: model.embedImages
                postImagePreview.embedAlts: model.embedImagesAlt
                postImagePreview.onRequestViewImages: (index) => requestViewImages(index, model.embedImagesFull, model.embedImagesAlt)

                quoteFilterFrame.visible: model.quoteFilterMatched && !model.quoteRecordBlocked
                quoteFilterFrame.labelText: qsTr("Quoted content warning")
                blockedQuoteFrame.visible: model.quoteRecordBlocked
                hasQuote: model.hasQuoteRecord && !model.quoteRecordBlocked
                quoteRecordFrame.onClicked: (mouse) => {
                                                if(model.quoteRecordUri.length > 0){
                                                    requestViewThread(model.quoteRecordUri)
                                                }
                                            }
                quoteRecordAvatarImage.source: model.quoteRecordAvatar
                quoteRecordAuthor.displayName: model.quoteRecordDisplayName
                quoteRecordAuthor.handle: model.quoteRecordHandle
                quoteRecordAuthor.indexedAt: model.quoteRecordIndexedAt
                quoteRecordRecordText.text: model.quoteRecordRecordText
                quoteRecordImagePreview.embedImages: model.quoteRecordEmbedImages
                quoteRecordImagePreview.embedAlts: model.quoteRecordEmbedImagesAlt
                quoteRecordImagePreview.onRequestViewImages: (index) => requestViewImages(index, model.quoteRecordEmbedImagesFull, model.quoteRecordEmbedImagesAlt)

                externalLinkFrame.visible: model.hasExternalLink
                externalLinkFrame.onClicked: Qt.openUrlExternally(model.externalLinkUri)
                externalLinkFrame.thumbImage.source: model.externalLinkThumb
                externalLinkFrame.titleLabel.text: model.externalLinkTitle
                externalLinkFrame.uriLabel.text: model.externalLinkUri
                externalLinkFrame.descriptionLabel.text: model.externalLinkDescription

                feedGeneratorFrame.visible: model.hasFeedGenerator
                feedGeneratorFrame.onClicked: postThreadView.requestViewFeedGenerator(model.feedGeneratorDisplayName, model.feedGeneratorUri)
                feedGeneratorFrame.avatarImage.source: model.feedGeneratorAvatar
                feedGeneratorFrame.displayNameLabel.text: model.feedGeneratorDisplayName
                feedGeneratorFrame.creatorHandleLabel.text: model.feedGeneratorCreatorHandle
                feedGeneratorFrame.likeCountLabel.text: model.feedGeneratorLikeCount

                postInformation.visible: (postThreadUri === model.uri)
                postInformation.labelsLayout.model: postInformation.visible ? model.labels : []
                postInformation.languagesLayout.model: postInformation.visible ? model.languages : []
                postInformation.indexedAtLongLabel.text: postInformation.visible ? model.indexedAtLong : ""
                postInformation.viaTagLabel.text: (postInformation.visible && model.via.length > 0) ? ("via:" + model.via) : ""

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
                postControls.onTriggeredTranslate: postThreadListModel.translate(model.cid)
                postControls.isReposted: model.isReposted
                postControls.isLiked: model.isLiked
                postControls.postUri: model.uri
                postControls.handle: model.handle
                postControls.mine: model.did === postThreadView.accountDid
                postControls.onTriggeredCopyToClipboard: systemTool.copyToClipboard(model.recordTextPlain)
                postControls.onTriggeredDeletePost: rootListView.model.deletePost(model.index)
                postControls.onTriggeredRequestReport: postThreadView.requestReportPost(model.uri, model.cid)
                postControls.onTriggeredRequestViewLikedBy: postThreadView.requestViewLikedBy(model.uri)
                postControls.onTriggeredRequestViewRepostedBy: postThreadView.requestViewRepostedBy(model.uri)

                threadConnectorTop.visible:  model.threadConnectorTop
                threadConnectorBottom.visible: model.threadConnectorBottom

                onHoveredLinkChanged: postThreadView.hoveredLink = hoveredLink
            }
        }
    }
}
