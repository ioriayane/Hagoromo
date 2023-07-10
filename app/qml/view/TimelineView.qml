import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.timelinelistmodel 1.0
import tech.relog.hagoromo.systemtool 1.0

import "../parts"
import "../controls"

ScrollView {
    id: timelineView
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    clip: true

    property string hoveredLink: ""
    property real fontSizeRatio: 1.0
    property string accountDid: ""   // 取得するユーザー

    property alias listView: rootListView
    property alias model: rootListView.model

    signal requestReply(string cid, string uri,
                        string reply_root_cid, string reply_root_uri,
                        string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestQuote(string cid, string uri, string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestViewThread(string uri)
    signal requestViewImages(int index, var paths)
    signal requestViewProfile(string did)


    ListView {
        id: rootListView
        anchors.fill: parent
        anchors.rightMargin: parent.ScrollBar.vertical.width
        spacing: 5

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

        delegate: PostDelegate {
            Layout.preferredWidth: rootListView.width

            fontSizeRatio: timelineView.fontSizeRatio

            onClicked: (mouse) => requestViewThread(model.uri)
            onRequestViewProfile: (did) => timelineView.requestViewProfile(did)

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
            postAuthor.indexedAt: model.indexedAt
            recordText.text: {
                var text = model.recordText
                if(model.recordTextTranslation.length > 0){
                    text = text + "<br/>---<br/>" + model.recordTextTranslation
                }
                return text
            }
            postImagePreview.embedImages: model.embedImages
            postImagePreview.onRequestViewImages: (index) => requestViewImages(index, model.embedImagesFull)

            childFrame.visible: model.hasQuoteRecord
            childFrame.onClicked: (mouse) => {
                                      if(model.quoteRecordUri.length > 0){
                                          requestViewThread(model.quoteRecordUri)
                                      }
                                  }
            childAvatarImage.source: model.quoteRecordAvatar
            childAuthor.displayName: model.quoteRecordDisplayName
            childAuthor.handle: model.quoteRecordHandle
            childAuthor.indexedAt: model.quoteRecordIndexedAt
            childRecordText.text: model.quoteRecordRecordText

            externalLinkFrame.visible: model.hasExternalLink
            externalLinkFrame.onClicked: Qt.openUrlExternally(model.externalLinkUri)
            externalLinkThumbImage.source: model.externalLinkThumb
            externalLinkTitleLabel.text: model.externalLinkTitle
            externalLinkUriLabel.text: model.externalLinkUri
            externalLinkDescriptionLabel.text: model.externalLinkDescription

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
            postControls.mine: model.did === timelineView.accountDid
            postControls.onTriggeredCopyToClipboard: systemTool.copyToClipboard(model.recordTextPlain)
            postControls.onTriggeredDeletePost: rootListView.model.deletePost(model.index)

            onHoveredLinkChanged: timelineView.hoveredLink = hoveredLink
        }
    }
}
