import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import "../controls"

ClickableFrame {
    id: postFrame
    topPadding: 10
    leftPadding: 10
    rightPadding: 10
    bottomPadding: 2
    style: "Post"

    property int layoutWidth: postFrame.Layout.preferredWidth
    property string hoveredLink: ""
    property real fontSizeRatio: 1.0
    property bool userFilterMatched: false
    property string userFilterMessage: ""
    property bool hasQuote: false

    property alias moderationFrame: moderationFrame
    property alias repostReactionAuthor: repostReactionAuthor
    property alias replyReactionAuthor: replyReactionAuthor
    property alias postAvatarImage: postAvatarImage
    property alias postAuthor: postAuthor
    property alias recordText: recordText
    property alias contentFilterFrame: contentFilterFrame
    property alias contentMediaFilterFrame: contentMediaFilterFrame
    property alias postImagePreview: postImagePreview
    property alias quoteFilterFrame: quoteFilterFrame
    property alias quoteRecordFrame: quoteRecordFrame
    property alias quoteRecordAvatarImage: quoteRecordAvatarImage
    property alias quoteRecordAuthor: quoteRecordAuthor
    property alias quoteRecordRecordText: quoteRecordRecordText
    property alias quoteRecordImagePreview: quoteRecordImagePreview
    property alias blockedQuoteFrame: blockedQuoteFrame
    property alias externalLinkFrame: externalLinkFrame
    property alias generatorViewFrame: generatorFeedFrame
    property alias generatorAvatarImage: generatorFeedAvatarImage
    property alias generatorDisplayNameLabel: generatorFeedDisplayNameLabel
    property alias generatorCreatorHandleLabel: generatorFeedCreatorHandleLabel
    property alias generatorLikeCountLabel: generatorFeedLikeCountLabel
    property alias labelsLayout: labelsLayout
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

    ColumnLayout {
        states: [
            State {
                when: moderationFrame.showContent === false
                PropertyChanges { target: repostReactionAuthor; visible: false }
                PropertyChanges { target: replyReactionAuthor; visible: false }
                PropertyChanges { target: postLayout; visible: false }
            }

        ]

        CoverFrame {
            id: moderationFrame
            Layout.preferredWidth: postFrame.layoutWidth - postFrame.leftPadding - postFrame.rightPadding
            Layout.bottomMargin: 8
            visible: false
            labelText: qsTr("Post from an account you muted.")
            states: [
                State {
                    when: postFrame.userFilterMatched
                    PropertyChanges {
                        target: moderationFrame
                        visible: true
                        labelText: postFrame.userFilterMessage
                    }
                }
            ]
        }

        ReactionAuthor {
            id: repostReactionAuthor
            Layout.maximumWidth: postFrame.layoutWidth
            source: "../images/repost.png"
            color: Material.color(Material.Green)
        }
        ReactionAuthor {
            id: replyReactionAuthor
            Layout.maximumWidth: postFrame.layoutWidth
            source: "../images/reply.png"
            color: Material.color(Material.Blue)
        }

        RowLayout {
            id: postLayout
            spacing: 10
            AvatarImage {
                id: postAvatarImage
                Layout.preferredWidth: 36
                Layout.preferredHeight: 36
                Layout.alignment: Qt.AlignTop
            }
            ColumnLayout {
                id: bodyLayout
                Layout.preferredWidth: basisWidth
                spacing: 0

                property int basisWidth: postFrame.layoutWidth - postFrame.leftPadding - postFrame.rightPadding -
                                         postLayout.spacing - postAvatarImage.Layout.preferredWidth

                Author {
                    id: postAuthor
                    layoutWidth: parent.basisWidth
                }

                CoverFrame {
                    id: contentFilterFrame
                    Layout.preferredWidth: parent.basisWidth
                    Layout.topMargin: 5
                    visible: false
                }

                ColumnLayout {
                    id: contentLayout
                    Layout.preferredWidth: parent.basisWidth
                    spacing: 0
                    visible: contentFilterFrame.showContent

                    Label {
                        id: recordText
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                        Layout.bottomMargin: -5
                        visible: text.length > 0
                        textFormat: Text.StyledText
                        wrapMode: Text.WrapAnywhere
                        font.pointSize: 10 * fontSizeRatio
                        lineHeight: 1.3
                        onLinkActivated: (url) => openLink(url)
                        onHoveredLinkChanged: displayLink(hoveredLink)
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

                    CoverFrame {
                        id: quoteFilterFrame
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                        visible: false
                    }
                    ClickableFrame {
                        id: quoteRecordFrame
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                        visible: postFrame.hasQuote && quoteFilterFrame.showContent
                        RowLayout {
                            id: quoteRecordLayout
                            spacing: 10
                            AvatarImage {
                                id: quoteRecordAvatarImage
                                Layout.preferredWidth: 16
                                Layout.preferredHeight: 16
                                Layout.alignment: Qt.AlignTop
                            }
                            ColumnLayout {
                                Layout.fillWidth: true
                                property int basisWidth: bodyLayout.basisWidth - quoteRecordFrame.padding * 2 -
                                                         quoteRecordLayout.spacing - quoteRecordAvatarImage.Layout.preferredWidth
                                Author {
                                    id: quoteRecordAuthor
                                    layoutWidth: parent.basisWidth
                                }
                                Label {
                                    id: quoteRecordRecordText
                                    Layout.preferredWidth: parent.basisWidth
                                    Layout.maximumWidth: parent.basisWidth
                                    visible: text.length > 0
                                    textFormat: Text.StyledText
                                    wrapMode: Text.WrapAnywhere
                                    font.pointSize: 10 * fontSizeRatio
                                    lineHeight: 1.3
                                    onLinkActivated: (url) => openLink(url)
                                    onHoveredLinkChanged: displayLink(hoveredLink)
                                }
                                ImagePreview {
                                    id: quoteRecordImagePreview
                                    layoutWidth: parent.basisWidth
                                    Layout.topMargin: 5
                                }
                            }
                        }
                    }
                    Frame {
                        id: blockedQuoteFrame
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                        visible: false
                        Label {
                            text: qsTr("blocked")
                        }
                    }

                    ExternalLinkCard {
                        id: externalLinkFrame
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                        hoverEnabled: true
                        onHoveredChanged:{
                            if(hovered){
                                displayLink(uriLabel.text)
                            }else{
                                displayLink("")
                            }
                        }
                    }

                    ClickableFrame {
                        id: generatorFeedFrame
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5

                        ColumnLayout {
                            GridLayout {
                                columns: 2
                                rowSpacing: 3
                                AvatarImage {
                                    id: generatorFeedAvatarImage
                                    Layout.preferredWidth: 24
                                    Layout.preferredHeight: 24
                                    Layout.rowSpan: 2
                                    altSource: "../images/account_icon.png"
                                }
                                Label {
                                    id: generatorFeedDisplayNameLabel
                                    Layout.fillWidth: true
                                    font.pointSize: 10
                                }
                                Label {
                                    id: generatorFeedCreatorHandleLabel
                                    color: Material.color(Material.Grey)
                                    font.pointSize: 8
                                }
                            }
                            RowLayout {
                                Layout.leftMargin: 3
                                spacing: 3
                                Image {
                                    Layout.preferredWidth: 16
                                    Layout.preferredHeight: 16
                                    source: "../images/like.png"
                                    layer.enabled: true
                                    layer.effect: ColorOverlay {
                                        color: Material.color(Material.Pink)
                                    }
                                }
                                Label {
                                    id: generatorFeedLikeCountLabel
                                    Layout.alignment: Qt.AlignVCenter
                                    Layout.fillWidth: true
                                    font.pointSize: 8
                                }
                            }
                        }
                    }

                    TagLabelLayout {
                        id: labelsLayout
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                        Layout.bottomMargin: 2
                        visible: count > 0
                    }

                    PostControls {
                        id: postControls
                    }
                }
            }
        }
    }
}
