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

    property alias model: rootListView.model

    signal requestedReply(string cid, string uri,
                          string reply_root_cid, string reply_root_uri,
                          string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestedRepost(string cid, string uri)
    signal requestedQuote(string cid, string uri, string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestedLike(string cid, string uri)

    ListView {
        id: rootListView
        anchors.fill: parent
        anchors.rightMargin: parent.ScrollBar.vertical.width

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

        delegate: Frame {
            id: postFrame
            width: rootListView.width
            topPadding: 10
            leftPadding: 10
            rightPadding: 10
            bottomPadding: 0

            ColumnLayout {
                ReactionAuthor {
                    visible: model.isRepostedBy
                    source: "../images/repost.png"
                    displayName: model.repostedByDisplayName
                    handle: model.repostedByHandle
                    color: Material.color(Material.Green)
                }
                ReactionAuthor {
                    visible: model.hasReply
                    source: "../images/reply.png"
                    displayName: model.replyParentDisplayName
                    handle: model.replyParentHandle
                    color: Material.color(Material.Blue)
                }

                RowLayout {
                    id: postLayout
                    spacing: 10
                    AvatarImage {
                        id: postImage
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 36
                        Layout.alignment: Qt.AlignTop
                        source: model.avatar
                    }
                    ColumnLayout {
                        id: bodyLayout
                        Layout.fillWidth: true
                        spacing: 5

                        property int basisWidth: postFrame.width - postFrame.leftPadding - postFrame.rightPadding -
                                                 postLayout.spacing - postImage.Layout.preferredWidth

                        Author {
                            Layout.maximumWidth: parent.basisWidth
                            displayName: model.displayName
                            handle: model.handle
                            indexedAt: model.indexedAt
                        }

                        Label {
                            id: recordText
                            Layout.preferredWidth: parent.basisWidth
                            Layout.maximumWidth: parent.basisWidth
                            // wrapMode: Text.Wrap
                            wrapMode: Text.WrapAnywhere
                            font.pointSize: 11
//                            font.family: "遊ゴシック"  // "メイリオ"  "BIZ UDPゴシック"
                            text: model.recordText
                        }
                        ImagePreview {
                            layoutWidth: recordText.width
                            embedImages: model.embedImages
                        }

                        Frame {
                            id: childFrame
                            visible: model.hasChildRecord
                            Layout.fillWidth: true
                            RowLayout {
                                id: childLayout
                                spacing: 10
                                AvatarImage {
                                    id: childImage
                                    Layout.preferredWidth: 16
                                    Layout.preferredHeight: 16
                                    Layout.alignment: Qt.AlignTop
                                    source: model.childRecordAvatar
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    property int basisWidth: recordText.width - childFrame.padding * 2 - childLayout.spacing - childImage.Layout.preferredWidth
                                    Author {
                                        Layout.maximumWidth: parent.basisWidth
                                        displayName: model.childRecordDisplayName
                                        handle: model.childRecordHandle
                                        indexedAt: model.childRecordIndexedAt
                                    }
                                    Label {
                                        id: childRecordText
                                        Layout.preferredWidth: parent.basisWidth
                                        Layout.maximumWidth: parent.basisWidth
                                        wrapMode: Text.WrapAnywhere
                                        font.pointSize: 10
                                        text: model.childRecordRecordText
                                    }
                                }
                            }
                        }

                        PostControls {
                            id: postControls
                            replyButton.iconText: model.replyCount
                            repostButton.iconText: model.repostCount
                            likeButton.iconText: model.likeCount

                            replyButton.onClicked: requestedReply(model.cid, model.uri,
                                                                  model.replyRootCid, model.replyRootUri,
                                                                  model.avatar, model.displayName, model.handle, model.indexedAt, model.recordText)
                            repostMenuItem.onTriggered: requestedRepost(model.cid, model.uri)
                            quoteMenuItem.onTriggered: requestedQuote(model.cid, model.uri,
                                                                      model.avatar, model.displayName, model.handle, model.indexedAt, model.recordText)
                            likeButton.onClicked: requestedLike(model.cid, model.uri)
                        }
                    }
                }
            }
        }
    }
}
