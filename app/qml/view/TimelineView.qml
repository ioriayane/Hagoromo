import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.timelinelistmodel 1.0

import "../parts"
import "../controls"

ScrollView {
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    clip: true

    property alias model: rootListView.model
    ListView {
        id: rootListView
        anchors.fill: parent
        anchors.rightMargin: parent.ScrollBar.vertical.width

        model: TimelineListModel {
            id: timelineListModel
        }

        header: Button {
            width: rootListView.width
            height: 24
            display: AbstractButton.IconOnly
            icon.source: "../images/expand_less.png"
            onClicked: timelineListModel.getLatest()
        }
        footer: Button {
            width: rootListView.width
            height: 24
            display: AbstractButton.IconOnly
            icon.source: "../images/expand_more.png"
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
                    visible: model.hasParent
                    source: "../images/reply.png"
                    displayName: model.parentDisplayName
                    handle: model.parentHandle
                    color: Material.color(Material.Blue)
                }

                RowLayout {
                    id: postLayout
                    spacing: 10
                    Image {
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
                                Image {
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
                            replyButtonText: model.replyCount
                            repostButtonText: model.repostCount
                            likeButtonText: model.likeCount
                        }
                    }
                }
            }
        }
    }
}
