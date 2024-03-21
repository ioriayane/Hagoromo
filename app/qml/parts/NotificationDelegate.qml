import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.notificationlistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

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
    property alias postAuthor: postAuthor
    property alias contentFilterFrame: contentFilterFrame
    property alias contentMediaFilterFrame: contentMediaFilterFrame
    property alias postImagePreview: postImagePreview
    property alias quoteRecordFrame: quoteRecordFrame
    property alias quoteRecordImagePreview: quoteRecordImagePreview
    property alias feedGeneratorFrame: feedGeneratorFrame
    property alias listLinkCardFrame: listLinkCardFrame
    property alias externalLinkFrame: externalLinkFrame
    property alias postControls: postControls

    signal requestViewProfile(string did)
    signal requestViewSearchPosts(string text)
    signal requestAddMutedWord(string text)

    function openLink(url){
        if(url.indexOf("did:") === 0){
            requestViewProfile(url)
        }else if(url.indexOf("search://") === 0){
            tagMenu.x = recrdTextMouseArea.mouseX
            tagMenu.y = recrdTextMouseArea.mouseY
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
        }

    ]

    ColumnLayout {
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
                        Layout.preferredWidth: AdjustedValues.i16
                        Layout.preferredHeight: AdjustedValues.i16
                    }
                    Author {
                        id: postAuthor
                        layoutWidth: bodyLayout.basisWidth - postAvatarImage.width
                    }
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
                            onLinkActivated: (url) => openLink(url)
                            onHoveredLinkChanged: displayLink(hoveredLink)

                            HashTagMenu {
                                id: tagMenu
                                onRequestViewSearchPosts: (text) => notificationFrame.requestViewSearchPosts(text)
                                onRequestAddMutedWord: (text) => notificationFrame.requestAddMutedWord(text)
                            }
                        }
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
                        Layout.topMargin: 5
                        visible: contentMediaFilterFrame.showContent
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

                        property int basisWidth: parent.width - padding * 2

                        ColumnLayout {
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
                                onLinkActivated: (url) => openLink(url)
                                onHoveredLinkChanged: displayLink(hoveredLink)
                            }
                            ImagePreview {
                                id: quoteRecordImagePreview
                                layoutWidth: quoteRecordFrame.basisWidth
                                Layout.topMargin: 5
                            }
                        }
                    }

                    PostControls {
                        id: postControls
                        visible: false
                    }
                }
            }
        }
    }
}
