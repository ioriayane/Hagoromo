import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.timelinelistmodel 1.0

import "parts"

ScrollView {
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    property alias model: timelineListView.model
    ListView {
        id: timelineListView
        anchors.fill: parent
        anchors.rightMargin: parent.ScrollBar.vertical.width

        model: TimelineListModel {
            id: timelineListModel
        }

        header: Button {
            width: timelineListView.width
            height: 24
            display: AbstractButton.IconOnly
            icon.source: "images/expand_less.png"
            onClicked: timelineListModel.getLatest()
        }
        footer: Button {
            width: timelineListView.width
            height: 24
            display: AbstractButton.IconOnly
            icon.source: "images/expand_more.png"
        }

        delegate: Frame {
            id: postFrame
            padding: 10

            ColumnLayout {
                RowLayout {
                    visible: model.hasParent
                    Image {
                        Layout.preferredWidth: 12
                        Layout.preferredHeight: 12
                        source: "images/reply.png"
                        layer.enabled: true
                        layer.effect: ColorOverlay {
                            color: Material.color(Material.Blue)
                        }
                    }
                    Label {
                        Layout.alignment: Qt.AlignCenter
                        text: model.parentDisplayName
                        font.pointSize: 8
                        color: Material.color(Material.Blue)
                    }
                    Label {
                        Layout.alignment: Qt.AlignCenter
                        text: model.parentHandle
                        font.pointSize: 8
                        color: Material.color(Material.Blue)
                    }
                }
                RowLayout {
                    visible: model.isRepostedBy
                    Image {
                        Layout.preferredWidth: 12
                        Layout.preferredHeight: 12
                        source: "images/repost.png"
                        layer.enabled: true
                        layer.effect: ColorOverlay {
                            color: Material.color(Material.Green)
                        }
                    }
                    Label {
                        Layout.alignment: Qt.AlignCenter
                        text: model.repostedByDisplayName
                        font.pointSize: 8
                        color: Material.color(Material.Green)
                    }
                    Label {
                        Layout.alignment: Qt.AlignCenter
                        text: model.repostedByHandle
                        font.pointSize: 8
                        color: Material.color(Material.Green)
                    }
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

                        property int basisWidth: timelineListView.width - postFrame.padding * 2 - postLayout.spacing - postImage.Layout.preferredWidth

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

                        RowLayout {
                            IconButton {
                                iconSource: "images/reply.png"
                                iconText: model.replyCount
                            }
                            IconButton {
                                iconSource: "images/repost.png"
                                iconText: model.repostCount
                            }
                            IconButton {
                                iconSource: "images/like.png"
                                iconText: model.likeCount
                            }
                            Item {
                                Layout.fillWidth: true
                                height: 1
                            }
                            IconButton {
                                iconSource: "images/more.png"
                            }
                        }
                    }
                }
            }
        }
    }
}
