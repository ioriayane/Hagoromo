import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.timelinelistmodel 1.0
import tech.relog.hagoromo.notificationlistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../parts"
import "../compat"

ClickableFrame {
    id: notificationFrame
    contentWidth: contentRootLayout.implicitWidth
    contentHeight: contentRootLayout.implicitHeight
    topPadding: 10
    leftPadding: 10
    rightPadding: 10
    bottomPadding: 10
    style: "Post"

    property int layoutWidth: notificationFrame.Layout.preferredWidth

    property bool userFilterMatched: false
    property string userFilterMessage: ""

    property int reason: NotificationListModel.ReasonLike
    property string recordText: ""

    property string quoteRecordDisplayName: ""
    property string quoteRecordHandle: ""
    property string quoteRecordAvatar: ""
    property string quoteRecordIndexedAt: ""
    property string quoteRecordRecordText: ""

    property string hoveredLink: ""

    property alias moderationFrame: moderationFrame
    property alias postAvatarImage: postAvatarImage
    property alias aggregatedAvatarImages: aggregatedAvatarImages
    property alias postAuthor: postAuthor
    property alias skyblurContent: skyblurContent
    property alias contentFilterFrame: contentFilterFrame
    property alias contentMediaFilterFrame: contentMediaFilterFrame
    property alias postImagePreview: postImagePreview
    property alias embedVideoFrame: embedVideoFrame
    property alias quoteRecordFrame: quoteRecordFrame
    property alias quoteRecordImagePreview: quoteRecordImagePreview
    property alias quoteRecordEmbedVideoFrame: quoteRecordEmbedVideoFrame
    property alias blockedQuoteFrame: blockedQuoteFrame
    property alias blockedQuoteFrameLabel: blockedQuoteFrameLabel
    property alias feedGeneratorFrame: feedGeneratorFrame
    property alias listLinkCardFrame: listLinkCardFrame
    property alias externalLinkFrame: externalLinkFrame
    property alias postControls: postControls

    signal requestViewProfile(string did)
    signal requestViewSearchPosts(string text)
    signal requestAddMutedWord(string text)
    signal requestCopyTagToClipboard(string text)

    function openLink(url, x, y){
        if(url.indexOf("did:") === 0){
            requestViewProfile(url)
        }else if(url.indexOf("search://") === 0){
            tagMenu.x = x
            tagMenu.y = y
            tagMenu.tagText = url.substring(9)
            if(tagMenu.tagText.charAt(0) !== "#"){
                tagMenu.tagText = "#" + tagMenu.tagText
            }
            tagMenu.open()
        }else{
            Qt.openUrlExternally(url)
        }
    }

    function displayLink(url){
        if(url === undefined || url.indexOf("did:") === 0 || url.indexOf("search://") === 0){
            hoveredLink = ""
        }else{
            hoveredLink = url
        }
    }
    states: [
        State {
            when: notificationFrame.reason === NotificationListModel.ReasonLike &&
                  feedGeneratorFrame.visible === false
            PropertyChanges { target: reasonImage; source: "../images/like.png" }
            PropertyChanges { target: recrdTextMouseArea; visible: true }
            PropertyChanges { target: recordTextLabel; color: Material.color(Material.Grey) }
            PropertyChanges { target: recordTextLabel; text: notificationFrame.quoteRecordRecordText }
            PropertyChanges { target: notificationFrame; bottomPadding: 5 }
        },
        State {
            when: notificationFrame.reason === NotificationListModel.ReasonLike &&
                  feedGeneratorFrame.visible === true
            PropertyChanges { target: reasonImage; source: "../images/like.png" }
            PropertyChanges { target: recrdTextMouseArea; visible: false }
            PropertyChanges { target: recordTextLabel; color: Material.color(Material.Grey) }
            PropertyChanges { target: recordTextLabel; text: notificationFrame.quoteRecordRecordText }
            PropertyChanges { target: notificationFrame; bottomPadding: 5 }
        },
        State {
            when: notificationFrame.reason === NotificationListModel.ReasonRepost
            PropertyChanges { target: reasonImage; source: "../images/repost.png" }
            PropertyChanges { target: recrdTextMouseArea; visible: true }
            PropertyChanges { target: recordTextLabel; color: Material.color(Material.Grey) }
            PropertyChanges { target: recordTextLabel; text: notificationFrame.quoteRecordRecordText }
            PropertyChanges { target: notificationFrame; bottomPadding: 5 }
        },
        State {
            when: notificationFrame.reason === NotificationListModel.ReasonFollow
            PropertyChanges { target: reasonImage; source: "../images/add_user.png" }
        },
        State {
            when: notificationFrame.reason === NotificationListModel.ReasonMention
            PropertyChanges { target: reasonImage; source: "../images/reply.png" }
            PropertyChanges { target: recrdTextMouseArea; visible: true }
            PropertyChanges { target: recordTextLabel; text: notificationFrame.recordText }
            PropertyChanges { target: postControls; visible: true }
            PropertyChanges { target: notificationFrame; bottomPadding: 2 }
        },
        State {
            when: notificationFrame.reason === NotificationListModel.ReasonReply
            PropertyChanges { target: reasonImage; source: "../images/reply.png" }
            PropertyChanges { target: recrdTextMouseArea; visible: true }
            PropertyChanges { target: recordTextLabel; text: notificationFrame.recordText }
            PropertyChanges { target: postControls; visible: true }
            PropertyChanges { target: notificationFrame; bottomPadding: 2 }
        },
        State {
            when: notificationFrame.reason === NotificationListModel.ReasonQuote
            PropertyChanges { target: reasonImage; source: "../images/quote.png" }
            PropertyChanges { target: recrdTextMouseArea; visible: true }
            PropertyChanges { target: recordTextLabel; text: notificationFrame.recordText }
            // PropertyChanges { target: quoteRecordFrame; visible: true } ラベルフィルタの都合で使う側で調整する

            PropertyChanges { target: quoteRecordAvatarImage; source: notificationFrame.quoteRecordAvatar }
            PropertyChanges { target: quoteRecordAuthor; displayName: notificationFrame.quoteRecordDisplayName }
            PropertyChanges { target: quoteRecordAuthor; handle: notificationFrame.quoteRecordHandle }
            PropertyChanges { target: quoteRecordAuthor; indexedAt: notificationFrame.quoteRecordIndexedAt }
            PropertyChanges { target: quoteRecordRecordText; text: notificationFrame.quoteRecordRecordText }

            PropertyChanges { target: postControls; visible: true }
            PropertyChanges { target: notificationFrame; bottomPadding: 2 }
        },
        State {
            when: notificationFrame.reason === NotificationListModel.ReasonLikeViaRepost
            PropertyChanges { target: reasonImage; source: "../images/like_via_repost.png" }
            PropertyChanges { target: commonNotificationMessageText; visible: true; text: qsTr("Your repost has been liked.")}

            // PropertyChanges { target: quoteRecordFrame; visible: true } ラベルフィルタの都合で使う側で調整する
            PropertyChanges { target: quoteRecordAvatarImage; source: notificationFrame.quoteRecordAvatar }
            PropertyChanges { target: quoteRecordAuthor; displayName: notificationFrame.quoteRecordDisplayName }
            PropertyChanges { target: quoteRecordAuthor; handle: notificationFrame.quoteRecordHandle }
            PropertyChanges { target: quoteRecordAuthor; indexedAt: notificationFrame.quoteRecordIndexedAt }
            PropertyChanges { target: quoteRecordRecordText; text: notificationFrame.quoteRecordRecordText }

            PropertyChanges { target: notificationFrame; bottomPadding: 5 }
        },
        State {
            when: notificationFrame.reason === NotificationListModel.ReasonRepostViaRepost
            PropertyChanges { target: reasonImage; source: "../images/repost_via_repost.png" }
            PropertyChanges { target: commonNotificationMessageText; visible: true; text: qsTr("Your repost has been reposted.")}

            // PropertyChanges { target: quoteRecordFrame; visible: true } ラベルフィルタの都合で使う側で調整する
            PropertyChanges { target: quoteRecordAvatarImage; source: notificationFrame.quoteRecordAvatar }
            PropertyChanges { target: quoteRecordAuthor; displayName: notificationFrame.quoteRecordDisplayName }
            PropertyChanges { target: quoteRecordAuthor; handle: notificationFrame.quoteRecordHandle }
            PropertyChanges { target: quoteRecordAuthor; indexedAt: notificationFrame.quoteRecordIndexedAt }
            PropertyChanges { target: quoteRecordRecordText; text: notificationFrame.quoteRecordRecordText }

            PropertyChanges { target: notificationFrame; bottomPadding: 5 }
        },
        State {
            when: notificationFrame.reason === NotificationListModel.ReasonStaterPack
            PropertyChanges { target: reasonImage; source: "../images/starterpack.png" }
            PropertyChanges { target: commonNotificationMessageText; visible: true; text: qsTr("signed up with your starter pack")}
        }
    ]

    ColumnLayout {
        id: contentRootLayout
        spacing: 0

        CoverFrame {
            id: moderationFrame
            Layout.preferredWidth: notificationFrame.layoutWidth - notificationFrame.leftPadding - notificationFrame.rightPadding
            Layout.bottomMargin: showContent ? 8 : 4
            visible: false
            labelText: qsTr("Post from an account you muted.")
            states: [
                State {
                    when: notificationFrame.userFilterMatched
                    PropertyChanges {
                        target: moderationFrame
                        visible: true
                        labelText: notificationFrame.userFilterMessage
                    }
                }
            ]
        }

        RowLayout {
            id: postLayout
            visible: moderationFrame.showContent
            Image {
                id: reasonImage
                Layout.preferredWidth: AdjustedValues.i16
                Layout.preferredHeight: AdjustedValues.i16
                Layout.alignment: Qt.AlignTop
                source: "../images/like.png"
                layer.enabled: true
                layer.effect: ColorOverlayC {
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
                        },
                        State {
                            when: notificationFrame.reason === NotificationListModel.ReasonLikeViaRepost
                            PropertyChanges { target: reasonImageEffect; color: Material.color(Material.Pink) }
                        },
                        State {
                            when: notificationFrame.reason === NotificationListModel.ReasonRepostViaRepost
                            PropertyChanges { target: reasonImageEffect; color: Material.color(Material.Green) }
                        },
                        State {
                            when: notificationFrame.reason === NotificationListModel.ReasonStaterPack
                            PropertyChanges { target: reasonImageEffect; color: Material.color(Material.LightBlue) }
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
                    visible: !aggregatedAvatarImages.visible
                    AvatarImage {
                        id: postAvatarImage
                        Layout.preferredWidth: AdjustedValues.i16
                        Layout.preferredHeight: AdjustedValues.i16
                    }
                    Author {
                        id: postAuthor
                        layoutWidth: bodyLayout.basisWidth - postAvatarImage.width
                    }
                }
                AggregatedAvatarImages {
                    id: aggregatedAvatarImages
                    Layout.preferredWidth: parent.basisWidth
                    Layout.preferredHeight: height
                    visible: avatars.length > 1
                }

                CoverFrame {
                    id: contentFilterFrame
                    Layout.preferredWidth: parent.basisWidth
                    Layout.topMargin: 5
                    Layout.bottomMargin: showContent ? 0 : 5
                    visible: false
                }
                ColumnLayout {
                    id: contentLayout
                    Layout.preferredWidth: parent.basisWidth
                    spacing: 0
                    visible: contentFilterFrame.showContent

                    Label {
                        id: commonNotificationMessageText
                        Layout.preferredWidth: parent.width
                        Layout.leftMargin: postAvatarImage.width + 5
                        visible: false
                        font.pointSize: AdjustedValues.f8
                        color: Material.color(Material.Grey)
                    }

                    MouseArea {
                        id: recrdTextMouseArea
                        Layout.preferredWidth: parent.width
                        Layout.preferredHeight: recordTextLabel.contentHeight
                        Layout.topMargin: 5
                        Layout.bottomMargin: -5
                        acceptedButtons: Qt.MiddleButton
                        hoverEnabled: true
                        visible: false
                        Label {
                            id: recordTextLabel
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            textFormat: Text.StyledText
                            wrapMode: Text.WrapAnywhere
                            font.pointSize: AdjustedValues.f10
                            lineHeight: 1.3
                            onLinkActivated: (url) => openLink(url, recrdTextMouseArea.mouseX, recrdTextMouseArea.mouseY)
                            onHoveredLinkChanged: displayLink(hoveredLink)

                            HashTagMenu {
                                id: tagMenu
                                onRequestViewSearchPosts: (text) => notificationFrame.requestViewSearchPosts(text)
                                onRequestAddMutedWord: (text) => notificationFrame.requestAddMutedWord(text)
                                onRequestCopyTagToClipboard: (text) => notificationFrame.requestCopyTagToClipboard(text)
                            }
                        }
                    }

                    SkyblurContent {
                        id: skyblurContent
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                        visible: false
                    }

                    CoverFrame {
                        id: contentMediaFilterFrame
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                        visible: false
                    }
                    ImagePreview {
                        id: postImagePreview
                        layoutWidth: parent.width
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                    }

                    VideoFrame {
                        id: embedVideoFrame
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                    }

                    ExternalLinkCard {
                        id: externalLinkFrame
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                        visible: false
                        hoverEnabled: true
                        onHoveredChanged:{
                            if(hovered){
                                displayLink(uriLabel.text)
                            }else{
                                displayLink("")
                            }
                        }
                    }

                    FeedGeneratorLinkCard {
                        id: feedGeneratorFrame
                        Layout.preferredWidth: parent.width
                        visible: false
                    }
                    ListLinkCard {
                        id: listLinkCardFrame
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                    }

                    ClickableFrame {
                        id: quoteRecordFrame
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                        visible: false
                        contentWidth: quoteRecordContentLayout.implicitWidth
                        contentHeight: quoteRecordContentLayout.implicitHeight

                        property int basisWidth: parent.width - padding * 2

                        ColumnLayout {
                            id: quoteRecordContentLayout
                            RowLayout {
                                id: quoteRecordAuthorLayout
                                AvatarImage {
                                    id: quoteRecordAvatarImage
                                    Layout.preferredWidth: AdjustedValues.i16
                                    Layout.preferredHeight: AdjustedValues.i16
                                }
                                Author {
                                    id: quoteRecordAuthor
                                    layoutWidth: quoteRecordFrame.basisWidth - quoteRecordAvatarImage.Layout.preferredWidth - quoteRecordAuthorLayout.spacing
                                }
                            }
                            Label {
                                id: quoteRecordRecordText
                                Layout.preferredWidth: quoteRecordFrame.basisWidth
                                textFormat: Text.StyledText
                                wrapMode: Text.WrapAnywhere
                                font.pointSize: AdjustedValues.f10
                                lineHeight: 1.3
                                onLinkActivated: (url) => openLink(url,
                                                                   quoteRecordRecordText.x + quoteRecordRecordText.width / 4,
                                                                   quoteRecordRecordText.y + quoteRecordRecordText.height / 2)
                                onHoveredLinkChanged: displayLink(hoveredLink)
                            }
                            ImagePreview {
                                id: quoteRecordImagePreview
                                layoutWidth: quoteRecordFrame.basisWidth
                                Layout.topMargin: 5
                            }
                            VideoFrame {
                                id: quoteRecordEmbedVideoFrame
                                Layout.preferredWidth: parent.width
                                Layout.topMargin: 5
                            }
                        }
                    }
                    Frame {
                        id: blockedQuoteFrame
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                        visible: false
                        Label {
                            id: blockedQuoteFrameLabel
                            font.pointSize: AdjustedValues.f10
                        }
                    }

                    PostControls {
                        id: postControls
                        Layout.preferredWidth: parent.width
                        visible: false
                    }
                }
            }
        }
    }
}
