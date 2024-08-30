import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.timelinelistmodel 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"
import "../compat"

ScrollView {
    id: timelineView
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    clip: true

    property string hoveredLink: ""
    property string accountDid: ""   // 取得するユーザー
    property int imageLayoutType: 1
    property bool logMode: false

    property alias listView: rootListView
    property alias model: rootListView.model
    property var blogModel: undefined

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

    signal scrollPositionChanged(bool top)

    ListView {
        id: rootListView
        anchors.fill: parent
        anchors.rightMargin: parent.ScrollBar.vertical.width
        spacing: 5
        maximumFlickVelocity: AdjustedValues.maximumFlickVelocity

        onAtYBeginningChanged: scrollPositionChanged(atYBeginning)
        onMovementStarted: scrollPositionChanged(atYBeginning)
        onMovementEnded: {
            if(atYEnd){
                rootListView.model.getNext()
            }
        }

        SystemTool {
            id: systemTool
        }

        header: ColumnLayout {
            spacing: 0
            BlogEntryListView {
                id: blogListView
                Layout.preferredWidth: rootListView.width
                Layout.preferredHeight: (currentItem ? currentItem.height : 50)
                visible: timelineView.blogModel !== undefined && blogListView.count > 0
                model: timelineView.blogModel
                onHoveredLinkChanged: timelineView.hoveredLink = hoveredLink
            }
            ItemDelegate {
                Layout.preferredWidth: rootListView.width
                Layout.preferredHeight: AdjustedValues.h24
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
        }
        footer: BusyIndicator {
            width: rootListView.width
            height: AdjustedValues.i24
            visible: rootListView.model.running && rootListView.model.rowCount() > 0
        }

        //        add: Transition {
        //            NumberAnimation { properties: "x"; from: rootListView.width; duration: 300 }
        //        }

        delegate: PostDelegate {
            Layout.preferredWidth: rootListView.width

            logMode: timelineView.logMode

            onClicked: (mouse) => requestViewThread(model.uri)
            onRequestViewProfile: (did) => timelineView.requestViewProfile(did)
            onRequestViewSearchPosts: (text) => timelineView.requestViewSearchPosts(text)
            onRequestAddMutedWord: (text) => timelineView.requestAddMutedWord(text)

            moderationFrame.visible: model.muted
            userFilterMatched: model.userFilterMatched
            userFilterMessage: model.userFilterMessage

            repostReactionAuthor.visible: model.isRepostedBy
            repostReactionAuthor.displayName: model.repostedByDisplayName
            repostReactionAuthor.handle: model.repostedByHandle
            replyReactionAuthor.visible: model.hasReply
            replyReactionAuthor.displayName: model.replyParentDisplayName
            replyReactionAuthor.handle: model.replyParentHandle
            pinnedIndicatorLabel.visible: (model.pinned && model.index === 0)

            postAvatarImage.source: model.avatar
            postAvatarImage.onClicked: requestViewProfile(model.did)
            postAuthor.displayName: model.displayName
            postAuthor.handle: model.handle
            postAuthor.indexedAt: timelineView.logMode ? model.indexedAtLong : model.indexedAt
            recordText.text: {
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
            postImagePreview.layoutType: timelineView.imageLayoutType
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
            quoteRecordImagePreview.layoutType: timelineView.imageLayoutType
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

            postControls.repostCount: model.repostCount
            postControls.quoteCount: model.quoteCount
            postControls.replyButton.iconText: model.replyCount
            postControls.repostButton.iconText: model.repostCount + (model.quoteCount > 0 ? "+" + model.quoteCount : "")
            postControls.likeButton.iconText: model.likeCount
            postControls.replyButton.enabled: !model.replyDisabled
            postControls.repostButton.enabled: !model.runningRepost
            postControls.likeButton.enabled: !model.runningLike
            postControls.pinnedImage.enabled: !model.runningPostPinning
            postControls.quoteMenuItem.enabled: !model.quoteDisabled
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
            postControls.pinned: model.pinnedByMe
            postControls.threadMuted: model.threadMuted
            postControls.postUri: model.uri
            postControls.handle: model.handle
            postControls.mine: model.did === timelineView.accountDid
            postControls.logMode: timelineView.logMode
            postControls.onTriggeredCopyToClipboard: systemTool.copyToClipboard(model.recordTextPlain)
            postControls.onTriggeredCopyPostUrlToClipboard: systemTool.copyToClipboard(rootListView.model.getItemOfficialUrl(model.index))
            postControls.onTriggeredDeletePost: rootListView.model.deletePost(model.index)
            postControls.onTriggeredRequestReport: timelineView.requestReportPost(model.uri, model.cid)
            postControls.onTriggeredRequestViewLikedBy: timelineView.requestViewLikedBy(model.uri)
            postControls.onTriggeredRequestViewRepostedBy: timelineView.requestViewRepostedBy(model.uri)
            postControls.onTriggeredRequestViewQuotes: timelineView.requestViewQuotes(model.uri)
            postControls.onTriggeredRequestUpdateThreadGate: timelineView.requestUpdateThreadGate(model.uri, model.threadGateUri, model.threadGateType, model.threadGateRules, updatedThreadGate)
            postControls.onTriggeredRequestPin: rootListView.model.pin(model.index)
            postControls.onTriggeredRequestMuteThread: rootListView.model.muteThread(model.index)
            onHoveredLinkChanged: timelineView.hoveredLink = hoveredLink

            function updatedThreadGate(threadgate_uri, type, rules){
                console.log("updatedThreadGate:" + model.index + ", " + model.threadGateUri + ", threadgate_uri=" + threadgate_uri + ", " + type + ", rules=" + rules)
                rootListView.model.update(model.index, TimelineListModel.ThreadGateUriRole, threadgate_uri)
                rootListView.model.update(model.index, TimelineListModel.ThreadGateRulesRole, rules)
            }
        }
    }
}
