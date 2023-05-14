import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.postthreadlistmodel 1.0

import "../parts"
import "../controls"

ScrollView {
    id: postThreadView
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    clip: true

    property alias postThreadUri: postThreadListModel.postThreadUri
    property alias model: postThreadListModel

    signal requestedReply(string cid, string uri,
                          string reply_root_cid, string reply_root_uri,
                          string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestedRepost(string cid, string uri)
    signal requestedQuote(string cid, string uri, string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestedLike(string cid, string uri)
    signal requestedViewThread(string uri)
    signal requestedViewImages(int index, string paths)

    signal back()

    ListView {
        id: rootListView
        anchors.fill: parent
        anchors.rightMargin: parent.ScrollBar.vertical.width

        model: PostThreadListModel {
            id: postThreadListModel
            autoLoading: false
        }

        header: ItemDelegate {
            width: rootListView.width
            height: 24
            display: AbstractButton.IconOnly
            icon.source: postThreadListModel.running ? "" : "../images/arrow_back.png"
            onClicked: postThreadView.back()

            BusyIndicator {
                anchors.centerIn: parent
                width: 24
                height: 24
                visible: postThreadListModel.running
            }
        }

        delegate: PostDelegate {
            width: rootListView.width


            repostReactionAuthor.visible: model.isRepostedBy
            repostReactionAuthor.displayName: model.repostedByDisplayName
            repostReactionAuthor.handle: model.repostedByHandle
            replyReactionAuthor.visible: model.hasReply
            replyReactionAuthor.displayName: model.replyParentDisplayName
            replyReactionAuthor.handle: model.replyParentHandle

            postAvatarImage.source: model.avatar
            postAuthor.displayName: model.displayName
            postAuthor.handle: model.handle
            postAuthor.indexedAt: model.indexedAt
            recordText.text: model.recordText
            recordTextMouseArea.onClicked: requestedViewThread(model.uri)
            postImagePreview.embedImages: model.embedImages
            postImagePreview.onRequestedViewImages: (index) => requestedViewImages(index, model.embedImagesFull)

            childFrame.visible: model.hasChildRecord
            childAvatarImage.source: model.childRecordAvatar
            childAuthor.displayName: model.childRecordDisplayName
            childAuthor.handle: model.childRecordHandle
            childAuthor.indexedAt: model.childRecordIndexedAt
            childRecordText.text: model.childRecordRecordText

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
        }
    }
}
