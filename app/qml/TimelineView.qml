import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.timelinelistmodel 1.0

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
            padding: 10

            ColumnLayout {
                RowLayout {
                    visible: model.hasParent
                    Image {
                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 16
                        source: "images/reply.png"
                        layer.enabled: true
                        layer.effect: ColorOverlay {
                            color: Material.color(Material.Blue)
                        }
                    }
                    Label {
                        Layout.alignment: Qt.AlignCenter
                        text: model.parentDisplayName
                        font.pointSize: 10
                        color: Material.color(Material.Blue)
                    }
                }
                RowLayout {
                    visible: model.isRepostedBy
                    Image {
                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 16
                        source: "images/repost.png"
                        layer.enabled: true
                        layer.effect: ColorOverlay {
                            color: Material.color(Material.Green)
                        }
                    }
                    Label {
                        Layout.alignment: Qt.AlignCenter
                        text: model.repostedByDisplayName
                        font.pointSize: 10
                        color: Material.color(Material.Green)
                    }
                }
                RowLayout {
                    id: postLayout
                    spacing: 10
                    Image {
                        Layout.preferredWidth: 48
                        Layout.preferredHeight: 48
                        Layout.alignment: Qt.AlignTop
                        source: model.avatar
                    }
                    ColumnLayout {
                        id: bodyLayout
                        Layout.fillWidth: true
                        spacing: 5

                        RowLayout {
                            id: headerLayout
                            Layout.maximumWidth: parent.width
                            Label {
                                Layout.maximumWidth: headerLayout.width - handleText.width - indexAtText.width - headerLayout.spacing * 3
                                font.pointSize: 10
                                elide: Text.ElideRight
                                text: model.displayName
                            }
                            Label {
                                id: handleText
                                opacity: 0.8
                                font.pointSize: 8
                                text: model.handle
                            }
                            Item {
                                id: spacerLine1Item
                                Layout.fillWidth: true
                            }
                            Label {
                                id: indexAtText
                                Layout.minimumWidth: contentWidth
                                opacity: 0.8
                                font.pointSize: 8
                                text: model.indexedAt
                            }
                        }
                        Label {
                            id: recordText
                            Layout.preferredWidth: timelineListView.width - 30 - 48
                            // wrapMode: Text.Wrap
                            wrapMode: Text.WrapAnywhere
                            font.pointSize: 11
                            text: model.recordText
                        }
                        GridLayout {
                            id: imagePreviewLayout
                            visible: model.embedImages.length > 0
                            columnSpacing: 5
                            rowSpacing: 5
                            columns: 2
                            property string embedImages: model.embedImages
                            Repeater {
                                model: imagePreviewLayout.embedImages.split("\n")
                                delegate: Image {
                                    Layout.preferredWidth: recordText.width * 0.5 - 5
                                    Layout.preferredHeight: Layout.preferredWidth
                                    fillMode: Image.PreserveAspectCrop
                                    source: modelData
                                }
                            }
                        }

                        RowLayout {
                            //                    Layout.topMargin: 10
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
