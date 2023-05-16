import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.notificationlistmodel 1.0

import "../parts"

ScrollView {
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    clip: true

    property alias model: rootListView.model

    ListView {
        id: rootListView
        anchors.fill: parent
        anchors.rightMargin: parent.ScrollBar.vertical.width

        model: NotificationListModel {
            id: listNotificationListModel
        }

        header: ItemDelegate {
            width: rootListView.width
            height: 24
            display: AbstractButton.IconOnly
            icon.source: listNotificationListModel.running ? "" : "../images/expand_less.png"
            onClicked: listNotificationListModel.getLatest()

            BusyIndicator {
                anchors.centerIn: parent
                width: 24
                height: 24
                visible: listNotificationListModel.running
            }
        }
        footer: BusyIndicator {
            width: rootListView.width
            height: 24
            visible: listNotificationListModel.running && listNotificationListModel.rowCount() > 0
        }

        delegate: Frame {
            id: postFrame
            width: rootListView.width
            topPadding: 10
            leftPadding: 10
            rightPadding: 10
            bottomPadding: 10

            states: [
                State {
                    when: model.reason === NotificationListModel.ReasonLike
                    PropertyChanges { target: reasonImage; source: "../images/like.png" }
                    PropertyChanges { target: recordText; visible: true }
                    PropertyChanges { target: recordText; color: Material.color(Material.Grey) }
                    PropertyChanges { target: recordText; text: model.recordRecordText }
                },
                State {
                    when: model.reason === NotificationListModel.ReasonRepost
                    PropertyChanges { target: reasonImage; source: "../images/repost.png" }
                    PropertyChanges { target: recordText; visible: true }
                    PropertyChanges { target: recordText; color: Material.color(Material.Grey) }
                    PropertyChanges { target: recordText; text: model.recordRecordText }
                },
                State {
                    when: model.reason === NotificationListModel.ReasonFollow
                    PropertyChanges { target: reasonImage; source: "../images/add_user.png" }
                },
                State {
                    when: model.reason === NotificationListModel.ReasonMention
                    PropertyChanges { target: reasonImage; source: "../images/reply.png" }
                },
                State {
                    when: model.reason === NotificationListModel.ReasonReply
                    PropertyChanges { target: reasonImage; source: "../images/reply.png" }
                    PropertyChanges { target: recordText; visible: true }
                    PropertyChanges { target: recordText; text: model.recordText }
                    PropertyChanges { target: postControls; visible: true }
                },
                State {
                    when: model.reason === NotificationListModel.ReasonQuote
                    PropertyChanges { target: reasonImage; source: "../images/reply.png" }
                    PropertyChanges { target: postFrame; bottomPadding: 0 }
                    PropertyChanges { target: postControls; visible: true }
                }

            ]
            RowLayout {
                id: postLayout
                Image {
                    id: reasonImage
                    Layout.preferredWidth: 16
                    Layout.preferredHeight: 16
                    Layout.alignment: Qt.AlignTop
                    source: "../images/like.png"
                    layer.enabled: true
                    layer.effect: ColorOverlay {
                        id: reasonImageEffect
                        color: Material.color(Material.Grey)
                        states: [
                            State {
                                when: model.reason === NotificationListModel.ReasonLike
                                PropertyChanges { target: reasonImageEffect; color: Material.color(Material.Pink) }
                            },
                            State {
                                when: model.reason === NotificationListModel.ReasonRepost
                                PropertyChanges { target: reasonImageEffect; color: Material.color(Material.Green) }
                            },
                            State {
                                when: model.reason === NotificationListModel.ReasonFollow
                                PropertyChanges { target: reasonImageEffect; color: Material.color(Material.LightBlue) }
                            },
                            State {
                                when: model.reason === NotificationListModel.ReasonMention
                                PropertyChanges { target: reasonImageEffect; color: Material.color(Material.BlueGrey) }
                            },
                            State {
                                when: model.reason === NotificationListModel.ReasonReply
                                PropertyChanges { target: reasonImageEffect; color: Material.color(Material.Blue) }
                            },
                            State {
                                when: model.reason === NotificationListModel.ReasonQuote
                                PropertyChanges { target: reasonImageEffect; color: Material.color(Material.Lime) }
                            }
                        ]
                    }
                }

                ColumnLayout {
                    id: bodyLayout
                    Layout.fillWidth: true
                    spacing: 5

                    property int basisWidth: postFrame.width - postFrame.leftPadding - postFrame.rightPadding -
                                             postLayout.spacing - reasonImage.width - avatorImage.width

                    RowLayout {
                        AvatarImage {
                            id: avatorImage
                            Layout.preferredWidth: 16
                            Layout.preferredHeight: 16
                            source: model.avatar
                        }
                        Author {
                            Layout.preferredWidth: bodyLayout.basisWidth
                            Layout.maximumWidth: bodyLayout.basisWidth
                            //                            Layout.fillWidth: true
                            displayName: model.displayName
                            handle: model.handle
                            indexedAt: model.indexedAt
                        }
                    }

//                    Frame {
//                        id: recordFrame
//                        property int basisWidth: bodyLayout.width - padding * 2 -
//                                                 recordAvatarImage.width - recordAuthorLayout.spacing
//                        ColumnLayout {
//                            RowLayout {
//                                id: recordAuthorLayout
//                                AvatarImage {
//                                    id: recordAvatarImage
//                                    Layout.preferredWidth: 16
//                                    Layout.preferredHeight: 16
//                                    source: model.recordAvatar
//                                }
//                                Author {
//                                    Layout.preferredWidth: recordFrame.basisWidth
//                                    Layout.maximumWidth: recordFrame.basisWidth
//                                    //                            Layout.fillWidth: true
//                                    displayName: model.recordDisplayName
//                                    handle: model.recordHandle
//                                    indexedAt: model.recordIndexedAt
//                                }
//                            }
//                            Label {
//                                id: recordText
//                                Layout.preferredWidth: recordFrame.basisWidth
//                                Layout.maximumWidth: recordFrame.basisWidth
//                                wrapMode: Text.WrapAnywhere
//                                font.pointSize: 10
//                                lineHeight: 1.3
//                                text: model.recordRecordText
//                            }
//                        }

//                    }
                    Label {
                        id: recordText
                        visible: false
                        Layout.preferredWidth: bodyLayout.basisWidth
                        Layout.maximumWidth: bodyLayout.basisWidth
                        wrapMode: Text.WrapAnywhere
                        font.pointSize: 10
                        lineHeight: 1.3
                        text: model.recordRecordText
                    }

                    PostControls {
                        id: postControls
                        visible: false
                        replyButton.iconText: "0" //model.replyCount
                        repostButton.iconText: "0" //model.repostCount
                        likeButton.iconText: "0" //model.likeCount
                    }

                }
            }
        }
    }
}
