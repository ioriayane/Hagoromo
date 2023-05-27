import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.postthreadlistmodel 1.0

import "../parts"
import "../controls"

ColumnLayout {
    id: postThreadView

    property alias postThreadUri: postThreadListModel.postThreadUri
    property alias listView: rootListView
    property alias model: postThreadListModel

    signal requestReply(string cid, string uri,
                          string reply_root_cid, string reply_root_uri,
                          string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestRepost(string cid, string uri)
    signal requestQuote(string cid, string uri, string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestLike(string cid, string uri)
    signal requestViewThread(string uri)
    signal requestViewImages(int index, string paths)
    signal requestViewProfile(string did)

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
                text: "Post thread"
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

            model: PostThreadListModel {
                id: postThreadListModel
                autoLoading: false
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

            delegate: PostDelegate {
                width: rootListView.width

                //自分から自分へは移動しない
                //onClicked: (mouse) => requestViewThread(model.uri)

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
                        text = text + "\n---\n" + model.recordTextTranslation
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

                postControls.replyButton.iconText: model.replyCount
                postControls.repostButton.iconText: model.repostCount
                postControls.likeButton.iconText: model.likeCount
                postControls.replyButton.onClicked: requestReply(model.cid, model.uri,
                                                                   model.replyRootCid, model.replyRootUri,
                                                                   model.avatar, model.displayName, model.handle, model.indexedAt, model.recordText)
                postControls.repostMenuItem.onTriggered: requestRepost(model.cid, model.uri)
                postControls.quoteMenuItem.onTriggered: requestQuote(model.cid, model.uri,
                                                                       model.avatar, model.displayName, model.handle, model.indexedAt, model.recordText)
                postControls.likeButton.onClicked: requestLike(model.cid, model.uri)
                postControls.tranlateMenuItem.onTriggered: postThreadListModel.translate(model.cid)
            }
        }
    }
}
