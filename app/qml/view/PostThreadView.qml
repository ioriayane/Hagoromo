import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.timelinelistmodel 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"

ColumnLayout {
    id: postThreadView

    property string hoveredLink: ""
    property string accountDid: ""   // 取得するユーザー
    property int imageLayoutType: 1
    property string subTitle: ""
    property string postUri: ""

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
    signal requestViewQuotes(string uri)
    signal requestViewSearchPosts(string text)
    signal requestAddMutedWord(string text)
    signal requestUpdateThreadGate(string uri, string threadgate_uri, string type, var rules, var callback)
    signal requestReportPost(string uri, string cid)

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
                text: postThreadView.subTitle
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
            maximumFlickVelocity: AdjustedValues.maximumFlickVelocity

            SystemTool {
                id: systemTool
            }

            header: Item {
                width: rootListView.width
                height: AdjustedValues.h24

                BusyIndicator {
                    anchors.centerIn: parent
                    width: AdjustedValues.i24
                    height: AdjustedValues.i24
                    visible: rootListView.model.running
                }
            }

            //            add: Transition {
            //                NumberAnimation { properties: "x"; from: rootListView.width; duration: 300 }
            //            }

            delegate: PostDelegate {
                Layout.preferredWidth: rootListView.width

                property bool isBasisPost: (postThreadView.postUri === model.uri)

                //自分から自分へは移動しない
                onClicked: (mouse) => {
                               if(postThreadView.postUri !== model.uri){
                                   requestViewThread(model.uri)
                               }
                           }
                onRequestViewProfile: (did) => postThreadView.requestViewProfile(did)
                onRequestViewSearchPosts: (text) => postThreadView.requestViewSearchPosts(text)
                onRequestAddMutedWord: (text) => postThreadView.requestAddMutedWord(text)
                onRequestCopyTagToClipboard: (text) => systemTool.copyToClipboard(text)

                moderationFrame.visible: model.muted
                userFilterMatched: model.userFilterMatched
                userFilterMessage: model.userFilterMessage

                repostReactionAuthor.visible: model.isRepostedBy
                repostReactionAuthor.displayName: model.repostedByDisplayName
                repostReactionAuthor.handle: model.repostedByHandle
                replyReactionAuthor.visible: model.hasReply
                replyReactionAuthor.displayName: model.replyParentDisplayName
                replyReactionAuthor.handle: model.replyParentHandle
                pinnedIndicatorLabel.visible: false

                postAvatarImage.source: model.avatar
                postAvatarImage.onClicked: requestViewProfile(model.did)
                authorLiveIsActiveCover.visible: model.authorLiveIsActive
                postAuthor.displayName: model.displayName
                postAuthor.handle: model.handle
                postAuthor.indexedAt: isBasisPost ? "" : model.indexedAt
                postAuthor.verificationState: model.authorVerificationState
                authorLabels.model: isBasisPost ? model.authorLabels : []
                authorLabels.iconSources: isBasisPost ? model.authorLabelIcons : []
                recordText.text: {
                    var text = model.recordText
                    if(model.recordTextTranslation.length > 0){
                        text = text + "<br/>\n---<br/>\n" + model.recordTextTranslation
                    }
                    return text
                }
                skyblurContent.visible: model.hasSkyblurLink
                skyblurContent.postText.text: model.skyblurPostText
                skyblurContent.getPostTextButton.visible: (model.hasSkyblurLink && model.skyblurPostText.length === 0)
                skyblurContent.getPostTextButton.onClicked: rootListView.model.restoreBluredText(model.cid)
                skyblurContent.getPostTextButtonBusy.visible: model.runningSkyblurPostText

                pollContent.visible: model.hasPoll && contentMediaFilterFrame.showContent
                pollContent.options: model.pollOptions
                pollContent.countOfOptions: model.pollCountOfOptions
                pollContent.myVote: model.pollMyVote
                pollContent.totalVotes: model.pollTotalVotes
                pollContent.isEnded: model.pollIsEnded
                pollContent.remainTime: model.pollRemainTime

                contentFilterFrame.visible: model.contentFilterMatched
                contentFilterFrame.labelText: model.contentFilterMessage
                contentMediaFilterFrame.visible: model.contentMediaFilterMatched
                contentMediaFilterFrame.labelText: model.contentMediaFilterMessage
                postImagePreview.visible: contentMediaFilterFrame.showContent && model.embedImages.length > 0
                postImagePreview.layoutType: postThreadView.imageLayoutType
                postImagePreview.embedImages: model.embedImages
                postImagePreview.embedAlts: model.embedImagesAlt
                postImagePreview.embedImageRatios: model.embedImagesRatio
                postImagePreview.onRequestViewImages: (index) => requestViewImages(index, model.embedImagesFull, model.embedImagesAlt)

                quoteFilterFrame.visible: model.quoteFilterMatched && !model.quoteRecordBlocked
                quoteFilterFrame.labelText: qsTr("Quoted content warning")
                blockedQuoteFrame.visible: model.quoteRecordBlocked
                blockedQuoteFrameLabel.text: model.quoteRecordBlockedStatus
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
                quoteRecordImagePreview.layoutType: postThreadView.imageLayoutType
                quoteRecordImagePreview.embedImages: model.quoteRecordEmbedImages
                quoteRecordImagePreview.embedAlts: model.quoteRecordEmbedImagesAlt
                quoteRecordImagePreview.onRequestViewImages: (index) => requestViewImages(index, model.quoteRecordEmbedImagesFull, model.quoteRecordEmbedImagesAlt)
                quoteRecordFrame.quoteRecordEmbedVideoFrame.visible: model.quoteRecordHasVideo
                quoteRecordFrame.quoteRecordEmbedVideoFrame.thumbImageSource: model.quoteRecordVideoThumb
                quoteRecordFrame.quoteRecordEmbedVideoFrame.onClicked: Qt.openUrlExternally(rootListView.model.getItemOfficialUrl(model.index))
                embedVideoFrame.visible: contentMediaFilterFrame.showContent && model.hasVideo
                embedVideoFrame.onClicked: Qt.openUrlExternally(rootListView.model.getItemOfficialUrl(model.index))
                embedVideoFrame.thumbImageSource: model.videoThumbUri

                externalLinkFrame.visible: model.hasExternalLink && !model.hasPoll && contentMediaFilterFrame.showContent
                externalLinkFrame.onClicked: Qt.openUrlExternally(model.externalLinkUri)
                externalLinkFrame.thumbImage.source: model.externalLinkThumb
                externalLinkFrame.titleLabel.text: model.externalLinkTitle
                externalLinkFrame.uriLabel.text: model.externalLinkUri
                externalLinkFrame.descriptionLabel.text: model.externalLinkDescription

                feedGeneratorFrame.visible: model.hasFeedGenerator && contentMediaFilterFrame.showContent
                feedGeneratorFrame.onClicked: postThreadView.requestViewFeedGenerator(model.feedGeneratorDisplayName, model.feedGeneratorUri)
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

                postInformation.visible: isBasisPost
                postInformation.tagsLayout.model: postInformation.visible ? model.tags : []
                postInformation.labelsLayout.model: postInformation.visible ? model.labels : []
                postInformation.labelsLayout.iconSources: postInformation.visible ? model.labelIcons : []
                postInformation.languagesLayout.model: postInformation.visible ? model.languages : []
                postInformation.createdAtLongLabel.text: (model.isArchived && model.createdAt.length > 0) ? qsTr("Archived from %s").replace("%s", model.createdAt) : ""
                postInformation.indexedAtLongLabel.text: postInformation.visible ? model.indexedAtLong : ""
                postInformation.viaTagLabel.text: (postInformation.visible && model.via.length > 0) ? ("via:" + model.via) : ""

                postControls.repostCount: model.repostCount
                postControls.quoteCount: model.quoteCount
                postControls.replyButton.iconText: model.replyCount
                postControls.likeButton.iconText: model.likeCount
                postControls.bookmarkButton.iconText: model.bookmarkCount
                postControls.replyButton.enabled: !model.replyDisabled
                postControls.repostButton.enabled: !model.runningRepost
                postControls.likeButton.enabled: !model.runningLike
                postControls.bookmarkButton.enabled: !model.runningBookmark
                postControls.pinnedImage.enabled: !model.runningPostPinning
                postControls.moreButton.enabled: !model.runningOtherPrcessing
                postControls.quoteMenuItem.enabled: !model.quoteDisabled
                postControls.replyButton.onClicked: requestReply(model.cid, model.uri,
                                                                 model.replyRootCid, model.replyRootUri,
                                                                 model.avatar, model.displayName, model.handle, model.indexedAt, model.recordText)
                postControls.repostMenuItem.onTriggered: rootListView.model.repost(model.index)
                postControls.quoteMenuItem.onTriggered: requestQuote(model.cid, model.uri,
                                                                     model.avatar, model.displayName, model.handle, model.indexedAt, model.recordText)
                postControls.likeButton.onClicked: rootListView.model.like(model.index)
                postControls.bookmarkButton.onClicked: rootListView.model.bookmark(model.index)
                postControls.onTriggeredTranslate: rootListView.model.translate(model.cid)
                postControls.isReposted: model.isReposted
                postControls.isLiked: model.isLiked
                postControls.isBookmarked: model.isBookmarked
                postControls.pinned: model.pinnedByMe
                postControls.threadMuted: model.threadMuted
                postControls.quoteDetached: model.quoteRecordDetatched
                postControls.quoteIsMine: model.quoteRecordIsMine
                postControls.postUri: model.uri
                postControls.handle: model.handle
                postControls.mine: model.did === postThreadView.accountDid
                postControls.onTriggeredCopyToClipboard: systemTool.copyToClipboard(model.recordTextPlain)
                postControls.onTriggeredCopyPostUrlToClipboard: systemTool.copyToClipboard(rootListView.model.getItemOfficialUrl(model.index))
                postControls.onTriggeredDeletePost: rootListView.model.deletePost(model.index)
                postControls.onTriggeredRequestReport: postThreadView.requestReportPost(model.uri, model.cid)
                postControls.onTriggeredRequestViewLikedBy: postThreadView.requestViewLikedBy(model.uri)
                postControls.onTriggeredRequestViewRepostedBy: postThreadView.requestViewRepostedBy(model.uri)
                postControls.onTriggeredRequestViewQuotes: postThreadView.requestViewQuotes(model.uri)
                postControls.onTriggeredRequestUpdateThreadGate: postThreadView.requestUpdateThreadGate(model.uri, model.threadGateUri, model.threadGateType, model.threadGateRules, updatedThreadGate)
                postControls.onTriggeredRequestPin: rootListView.model.pin(model.index)
                postControls.onTriggeredRequestMuteThread: rootListView.model.muteThread(model.index)
                postControls.onTriggeredRequestDetachQuote: rootListView.model.detachQuote(model.index)

                threadConnected: model.threadConnected
                threadConnectorTop.visible:  model.threadConnectorTop
                threadConnectorBottom.visible: model.threadConnectorBottom

                onHoveredLinkChanged: postThreadView.hoveredLink = hoveredLink

                function updatedThreadGate(threadgate_uri, type, rules){
                    console.log("updatedThreadGate:" + model.index + ", " + model.threadGateUri + ", threadgate_uri=" + threadgate_uri + ", " + type + ", rules=" + rules)
                    rootListView.model.update(model.index, TimelineListModel.ThreadGateUriRole, threadgate_uri)
                    rootListView.model.update(model.index, TimelineListModel.ThreadGateRulesRole, rules)
                }
            }
        }
    }
}
