import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.postthreadlistmodel 1.0
import tech.relog.hagoromo.systemtool 1.0

import "../parts"
import "../controls"

ColumnLayout {
    id: postThreadView

    property string hoveredLink: ""
    property real fontSizeRatio: 1.0
    property string accountDid: ""   // 取得するユーザー

    property alias postThreadUri: postThreadListModel.postThreadUri
    property alias listView: rootListView
    property alias model: postThreadListModel

    signal requestReply(string cid, string uri,
                        string reply_root_cid, string reply_root_uri,
                        string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestQuote(string cid, string uri, string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestViewThread(string uri)
    signal requestViewImages(int index, var paths)
    signal requestViewProfile(string did)
    signal requestViewGeneratorFeed(string name, string uri)
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
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                flat: true
                iconSource: "../images/arrow_left_single.png"
                onClicked: postThreadView.back()
            }
            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 10
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

                onErrorOccured: (message) => {console.log(message)}
            }

            header: Item {
                width: rootListView.width
                height: 24

                BusyIndicator {
                    anchors.centerIn: parent
                    width: 24
                    height: 24
                    visible: postThreadListModel.running
                }
            }

            //            add: Transition {
            //                NumberAnimation { properties: "x"; from: rootListView.width; duration: 300 }
            //            }

            delegate: PostDelegate {
                Layout.preferredWidth: rootListView.width

                fontSizeRatio: postThreadView.fontSizeRatio

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
                postAuthor.indexedAt: model.indexedAt
                recordText.text: {
                    var text = model.recordText
                    if(model.recordTextTranslation.length > 0){
                        text = text + "<br/>\n---<br/>\n" + model.recordTextTranslation
                    }
                    return text
                }
                contentFilterFrame.visible: model.contentFilterMatched
                contentFilterFrame.labelText: model.contentFilterMessage
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
                externalLinkFrame.thumbImage.source: model.externalLinkThumb
                externalLinkFrame.titleLabel.text: model.externalLinkTitle
                externalLinkFrame.uriLabel.text: model.externalLinkUri
                externalLinkFrame.descriptionLabel.text: model.externalLinkDescription

                generatorViewFrame.visible: model.hasGeneratorFeed
                generatorViewFrame.onClicked: postThreadView.requestViewGeneratorFeed(model.generatorFeedDisplayName, model.generatorFeedUri)
                generatorAvatarImage.source: model.generatorFeedAvatar
                generatorDisplayNameLabel.text: model.generatorFeedDisplayName
                generatorCreatorHandleLabel.text: model.generatorFeedCreatorHandle
                generatorLikeCountLabel.text: model.generatorFeedLikeCount

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

                onHoveredLinkChanged: postThreadView.hoveredLink = hoveredLink
            }
        }
    }
}
