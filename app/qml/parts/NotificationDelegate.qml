import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.notificationlistmodel 1.0
import tech.relog.hagoromo.systemtool 1.0

import "../controls"
import "../parts"

ClickableFrame {
    id: notificationFrame
    topPadding: 10
    leftPadding: 10
    rightPadding: 10
    bottomPadding: 10
    style: "Post"

    property int layoutWidth: notificationFrame.Layout.preferredWidth

    property int reason: NotificationListModel.ReasonLike
    property string recordText: ""
    property string recordTextPlain: ""

    property string recordDisplayName: ""
    property string recordHandle: ""
    property string recordAvatar: ""
    property string recordIndexedAt: ""
    property string recordRecordText: ""

    property string hoveredLink: ""

    property alias postAvatarImage: postAvatarImage
    property alias postAuthor: postAuthor
    property alias recordFrame: recordFrame
    property alias recordImagePreview: recordImagePreview
    property alias postControls: postControls

    signal requestViewProfile(string did)

    function openLink(url){
        if(url.indexOf("did:") === 0){
            requestViewProfile(url)
        }else{
            Qt.openUrlExternally(url)
        }
    }

    function displayLink(url){
        if(url === undefined || url.indexOf("did:") === 0){
            hoveredLink = ""
        }else{
            hoveredLink = url
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
            PropertyChanges { target: recordTextLabel; visible: true }
            PropertyChanges { target: recordTextLabel; text: notificationFrame.recordText }
            PropertyChanges { target: postControls; visible: true }
            PropertyChanges { target: notificationFrame; bottomPadding: 2 }
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
            PropertyChanges { target: recordFrame; visible: true }

            PropertyChanges { target: recordAvatarImage; source: notificationFrame.recordAvatar }
            PropertyChanges { target: recordAuthor; displayName: notificationFrame.recordDisplayName }
            PropertyChanges { target: recordAuthor; handle: notificationFrame.recordHandle }
            PropertyChanges { target: recordAuthor; indexedAt: notificationFrame.recordIndexedAt }
            PropertyChanges { target: recordText; text: notificationFrame.recordRecordText }

            PropertyChanges { target: postControls; visible: true }
            PropertyChanges { target: notificationFrame; bottomPadding: 2 }
        }

    ]

    SystemTool {
        id: systemTool
    }

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
                        PropertyChanges { target: reasonImageEffect; color: Material.color(Material.Blue) }
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
            Layout.preferredWidth: basisWidth
            spacing: 0

            property int basisWidth: notificationFrame.layoutWidth - notificationFrame.leftPadding - notificationFrame.rightPadding -
                                     postLayout.spacing - reasonImage.width

            RowLayout {
                AvatarImage {
                    id: postAvatarImage
                    Layout.preferredWidth: 16
                    Layout.preferredHeight: 16
                }
                Author {
                    id: postAuthor
                    layoutWidth: bodyLayout.basisWidth - postAvatarImage.width
                }
            }

            Label {
                id: recordTextLabel
                visible: false
                Layout.preferredWidth: bodyLayout.basisWidth
                Layout.topMargin: 8
                textFormat: Text.StyledText
                wrapMode: Text.WrapAnywhere
                font.pointSize: 10
                lineHeight: 1.3
                onLinkActivated: (url) => openLink(url)
                onHoveredLinkChanged: displayLink(hoveredLink)
            }

            ClickableFrame {
                id: recordFrame
                Layout.preferredWidth: parent.basisWidth
                visible: false

                property int basisWidth: parent.basisWidth - padding * 2

                ColumnLayout {
                    RowLayout {
                        id: recordAuthorLayout
                        AvatarImage {
                            id: recordAvatarImage
                            Layout.preferredWidth: 16
                            Layout.preferredHeight: 16
                        }
                        Author {
                            id: recordAuthor
                            layoutWidth: recordFrame.basisWidth - recordAvatarImage.Layout.preferredWidth - recordAuthorLayout.spacing
                        }
                    }
                    Label {
                        id: recordText
                        Layout.preferredWidth: recordFrame.basisWidth
                        textFormat: Text.StyledText
                        wrapMode: Text.WrapAnywhere
                        font.pointSize: 10
                        lineHeight: 1.3
                        onLinkActivated: (url) => openLink(url)
                        onHoveredLinkChanged: displayLink(hoveredLink)
                    }
                    ImagePreview {
                        id: recordImagePreview
                        layoutWidth: recordFrame.basisWidth
                        Layout.topMargin: 5
                    }
                }
            }

            PostControls {
                id: postControls
                visible: false
                copyToClipboardMenuItem.onTriggered: systemTool.copyToClipboard(recordTextPlain)
            }
        }
    }
}
