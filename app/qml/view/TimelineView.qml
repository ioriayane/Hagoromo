import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.timelinelistmodel 1.0

import "../parts"
import "../controls"

ScrollView {
    id: timelineView
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    clip: true

    property alias listView: rootListView
    property alias model: rootListView.model

    signal requestedReply(string cid, string uri,
                          string reply_root_cid, string reply_root_uri,
                          string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestedRepost(string cid, string uri)
    signal requestedQuote(string cid, string uri, string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestedLike(string cid, string uri)
    signal requestedViewThread(string uri)
    signal requestedViewImages(int index, string paths)
    signal requestedViewProfile(string did)

    ListView {
        id: rootListView
        anchors.fill: parent
        anchors.rightMargin: parent.ScrollBar.vertical.width
        spacing: 5

        model: TimelineListModel {
            id: timelineListModel
            autoLoading: true
        }

        header: ItemDelegate {
            width: rootListView.width
            height: 24
            display: AbstractButton.IconOnly
            icon.source: timelineListModel.running ? "" : "../images/expand_less.png"
            onClicked: timelineListModel.getLatest()

            BusyIndicator {
                anchors.centerIn: parent
                width: 24
                height: 24
                visible: timelineListModel.running
            }
        }
        footer: BusyIndicator {
            width: rootListView.width
            height: 24
            visible: timelineListModel.running && timelineListModel.rowCount() > 0
        }

        delegate: PostDelegate {
            width: rootListView.width

            onClicked: (mouse) => requestedViewThread(model.uri)

            repostReactionAuthor.visible: model.isRepostedBy
            repostReactionAuthor.displayName: model.repostedByDisplayName
            repostReactionAuthor.handle: model.repostedByHandle
            replyReactionAuthor.visible: model.hasReply
            replyReactionAuthor.displayName: model.replyParentDisplayName
            replyReactionAuthor.handle: model.replyParentHandle

            postAvatarImage.source: model.avatar
            postAvatarImage.onClicked: requestedViewProfile(model.did)
            postAuthor.displayName: model.displayName
            postAuthor.handle: model.handle
            postAuthor.indexedAt: model.indexedAt
            recordText.text: {
                var text = model.recordText
                if(model.recordTextTranslation.length > 0){
                    text = text + "\n---\n" + model.recordTextTranslation
                }
                return text
            }
            postImagePreview.embedImages: model.embedImages
            postImagePreview.onRequestedViewImages: (index) => requestedViewImages(index, model.embedImagesFull)

            childFrame.visible: model.hasQuoteRecord
            childFrame.onClicked: (mouse) => {
                                      if(model.quoteRecordUri.length > 0){
                                          requestedViewThread(model.quoteRecordUri)
                                      }
                                  }
            childAvatarImage.source: model.quoteRecordAvatar
            childAuthor.displayName: model.quoteRecordDisplayName
            childAuthor.handle: model.quoteRecordHandle
            childAuthor.indexedAt: model.quoteRecordIndexedAt
            childRecordText.text: model.quoteRecordRecordText

            postControls.replyButton.iconText: model.replyCount
            postControls.repostButton.iconText: model.repostCount
            postControls.likeButton.iconText: model.likeCount
            postControls.replyButton.onClicked: requestedReply(model.cid, model.uri,
                                                               model.replyRootCid, model.replyRootUri,
                                                               model.avatar, model.displayName, model.handle, model.indexedAt, model.recordText)
            postControls.repostMenuItem.onTriggered: requestedRepost(model.cid, model.uri)
            postControls.quoteMenuItem.onTriggered: requestedQuote(model.cid, model.uri,
                                                                   model.avatar, model.displayName, model.handle, model.indexedAt, model.recordText)
            postControls.likeButton.onClicked: requestedLike(model.cid, model.uri)
            postControls.tranlateMenuItem.onTriggered: timelineListModel.translate(model.cid)
        }
    }
}
