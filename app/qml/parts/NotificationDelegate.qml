import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.notificationlistmodel 1.0

import "../parts"

Frame {
    id: notificationFrame
    width: rootListView.width
    topPadding: 10
    leftPadding: 10
    rightPadding: 10
    bottomPadding: 10

    property int reason: NotificationListModel.ReasonLike
    property string recordText: ""
    property string recordRecordText: ""

//    property alias repostReactionAuthor: repostReactionAuthor
//    property alias replyReactionAuthor: replyReactionAuthor
    property alias postAvatarImage: postAvatarImage
    property alias postAuthor: postAuthor
    property alias postFrameMouseArea: postFrameMouseArea
//    property alias postImagePreview: postImagePreview
//    property alias childFrame: childFrame
//    property alias childAvatarImage: childAvatarImage
//    property alias childAuthor: childAuthor
//    property alias childRecordText: childRecordText
    property alias postControls: postControls

    background: MouseArea {
        id: postFrameMouseArea
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.right: parent.right
            anchors.rightMargin: 5
            height: 1
            color: Material.color(Material.Grey, Material.Shade600)
        }
    }

    states: [
        State {
            when: notificationFrame.reason === NotificationListModel.ReasonLike
            PropertyChanges { target: reasonImage; source: "../images/like.png" }
            PropertyChanges { target: recordTextLabel; visible: true }
            PropertyChanges { target: recordTextLabel; color: Material.color(Material.Grey) }
            PropertyChanges { target: recordTextLabel; text: notificationFrame.recordRecordText }
            PropertyChanges { target: notificationFrame; bottomPadding: 5 }
        },
        State {
            when: notificationFrame.reason === NotificationListModel.ReasonRepost
            PropertyChanges { target: reasonImage; source: "../images/repost.png" }
            PropertyChanges { target: recordTextLabel; visible: true }
            PropertyChanges { target: recordTextLabel; color: Material.color(Material.Grey) }
            PropertyChanges { target: recordTextLabel; text: notificationFrame.recordRecordText }
            PropertyChanges { target: notificationFrame; bottomPadding: 5 }
        },
        State {
            when: notificationFrame.reason === NotificationListModel.ReasonFollow
            PropertyChanges { target: reasonImage; source: "../images/add_user.png" }
        },
        State {
            when: notificationFrame.reason === NotificationListModel.ReasonMention
            PropertyChanges { target: reasonImage; source: "../images/reply.png" }
        },
        State {
            when: notificationFrame.reason === NotificationListModel.ReasonReply
            PropertyChanges { target: reasonImage; source: "../images/reply.png" }
            PropertyChanges { target: recordTextLabel; visible: true }
            PropertyChanges { target: recordTextLabel; text: notificationFrame.recordText }
            PropertyChanges { target: postControls; visible: true }
            PropertyChanges { target: notificationFrame; bottomPadding: 2 }
        },
        State {
            when: notificationFrame.reason === NotificationListModel.ReasonQuote
            PropertyChanges { target: reasonImage; source: "../images/quote.png" }
            PropertyChanges { target: recordTextLabel; visible: true }
            PropertyChanges { target: recordTextLabel; text: notificationFrame.recordText }
            PropertyChanges { target: postControls; visible: true }
            PropertyChanges { target: notificationFrame; bottomPadding: 2 }
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
                        when: notificationFrame.reason === NotificationListModel.ReasonLike
                        PropertyChanges { target: reasonImageEffect; color: Material.color(Material.Pink) }
                    },
                    State {
                        when: notificationFrame.reason === NotificationListModel.ReasonRepost
                        PropertyChanges { target: reasonImageEffect; color: Material.color(Material.Green) }
                    },
                    State {
                        when: notificationFrame.reason === NotificationListModel.ReasonFollow
                        PropertyChanges { target: reasonImageEffect; color: Material.color(Material.LightBlue) }
                    },
                    State {
                        when: notificationFrame.reason === NotificationListModel.ReasonMention
                        PropertyChanges { target: reasonImageEffect; color: Material.color(Material.BlueGrey) }
                    },
                    State {
                        when: notificationFrame.reason === NotificationListModel.ReasonReply
                        PropertyChanges { target: reasonImageEffect; color: Material.color(Material.Blue) }
                    },
                    State {
                        when: notificationFrame.reason === NotificationListModel.ReasonQuote
                        PropertyChanges { target: reasonImageEffect; color: Material.color(Material.Lime) }
                    }
                ]
            }
        }

        ColumnLayout {
            id: bodyLayout
            Layout.fillWidth: true
            spacing: 0

            property int basisWidth: notificationFrame.width - notificationFrame.leftPadding - notificationFrame.rightPadding -
                                     postLayout.spacing - reasonImage.width - postAvatarImage.width

            RowLayout {
                AvatarImage {
                    id: postAvatarImage
                    Layout.preferredWidth: 16
                    Layout.preferredHeight: 16
                }
                Author {
                    id: postAuthor
                    Layout.preferredWidth: bodyLayout.basisWidth
                    Layout.maximumWidth: bodyLayout.basisWidth
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
                id: recordTextLabel
                visible: false
                Layout.preferredWidth: bodyLayout.basisWidth
                Layout.maximumWidth: bodyLayout.basisWidth
                Layout.topMargin: 8
                wrapMode: Text.WrapAnywhere
                font.pointSize: 10
                lineHeight: 1.3
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
