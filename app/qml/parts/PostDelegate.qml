import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0
import tech.relog.hagoromo.timelinelistmodel 1.0

import "../controls"

ClickableFrame {
    id: postFrame
    contentWidth: contentRootLayout.implicitWidth
    contentHeight: contentRootLayout.implicitHeight
    topPadding: 10
    leftPadding: 10
    rightPadding: 10
    bottomPadding: 2
    style: threadConnected ? "PostConnected" : "Post"

    property int layoutWidth: postFrame.Layout.preferredWidth
    property string hoveredLink: ""
    property bool userFilterMatched: false
    property string userFilterMessage: ""
    property bool hasQuote: false
    property bool threadConnected: false
    property bool logMode: false
    property int quoteRecordStatus: TimelineListModel.QuoteRecordBlocked

    property alias moderationFrame: moderationFrame
    property alias repostReactionAuthor: repostReactionAuthor
    property alias replyReactionAuthor: replyReactionAuthor
    property alias pinnedIndicatorLabel: pinnedIndicatorLabel
    property alias postAvatarImage: postAvatarImage
    property alias postAuthor: postAuthor
    property alias recordText: recordText
    property alias contentFilterFrame: contentFilterFrame
    property alias contentMediaFilterFrame: contentMediaFilterFrame
    property alias postImagePreview: postImagePreview
    property alias quoteFilterFrame: quoteFilterFrame
    property alias quoteRecordFrame: quoteRecordFrame
    property alias quoteRecordAvatarImage: quoteRecordFrame.quoteRecordAvatarImage
    property alias quoteRecordAuthor: quoteRecordFrame.quoteRecordAuthor
    property alias quoteRecordRecordText: quoteRecordFrame.quoteRecordRecordText
    property alias quoteRecordImagePreview: quoteRecordFrame.quoteRecordImagePreview
    property alias blockedQuoteFrame: blockedQuoteFrame
    property alias externalLinkFrame: externalLinkFrame
    property alias feedGeneratorFrame: feedGeneratorFrame
    property alias listLinkCardFrame: listLinkCardFrame
    property alias postInformation: postInformation
    property alias postControls: postControls
    property alias threadConnectorTop: threadConnectorTop
    property alias threadConnectorBottom: threadConnectorBottom

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

    ColumnLayout {
        id: contentRootLayout
        states: [
            State {
                when: moderationFrame.showContent === false
                PropertyChanges { target: repostReactionAuthor; visible: false }
                PropertyChanges { target: replyReactionAuthor; visible: false }
                PropertyChanges { target: pinnedIndicatorLabel; visible: false }
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

        PinnedIndicator {
            id: pinnedIndicatorLabel
            Layout.fillWidth: true
            Layout.preferredHeight: AdjustedValues.i12 * 1.2
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
                Layout.preferredWidth: AdjustedValues.i36
                Layout.preferredHeight: AdjustedValues.i36
                Layout.alignment: Qt.AlignTop
                clip: false
                Rectangle {
                    id: threadConnectorTop
                    x: (parent.width - width) / 2
                    y: -1 * postFrame.height
                    width: 2
                    height: postFrame.height - 2
                    visible: false
                    color: Material.color(Material.Grey)
                }
                Rectangle {
                    id: threadConnectorBottom
                    x: (parent.width - width) / 2
                    y: parent.height + 2
                    width: 2
                    height: postFrame.height
                    visible: false
                    color: Material.color(Material.Grey)
                }
            }
            ColumnLayout {
                id: bodyLayout
                Layout.preferredWidth: basisWidth
                spacing: 0

                property int basisWidth: postFrame.layoutWidth - postFrame.leftPadding - postFrame.rightPadding -
                                         postLayout.spacing - postAvatarImage.Layout.preferredWidth
                Author {
                    id: postAuthor
                    Layout.preferredWidth: parent.basisWidth
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

                    MouseArea {
                        id: recrdTextMouseArea
                        Layout.preferredWidth: parent.width
                        Layout.preferredHeight: recordText.contentHeight
                        Layout.topMargin: 5
                        Layout.bottomMargin: -5
                        acceptedButtons: Qt.MiddleButton
                        hoverEnabled: true
                        visible: recordText.text.length > 0
                        Label {
                            id: recordText
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
                                logMode: postFrame.logMode
                                onRequestViewSearchPosts: (text) => postFrame.requestViewSearchPosts(text)
                                onRequestAddMutedWord: (text) => postFrame.requestAddMutedWord(text)
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
                        Layout.preferredWidth: parent.width
                        layoutWidth: parent.width
                        Layout.topMargin: 5
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

                    FeedGeneratorLinkCard {
                        id: feedGeneratorFrame
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                    }
                    ListLinkCard {
                        id: listLinkCardFrame
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                    }

                    CoverFrame {
                        id: quoteFilterFrame
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                        visible: false
                    }
                    QuoteRecord {
                        id: quoteRecordFrame
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                        visible: postFrame.hasQuote &&
                                 quoteFilterFrame.showContent &&
                                 contentMediaFilterFrame.showContent
                        basisWidth: bodyLayout.basisWidth
                        onOpenLink: (url) => postFrame.openLink(url)
                        onDisplayLink: (url) => postFrame.displayLink(url)
                    }
                    Frame {
                        id: blockedQuoteFrame
                        Layout.preferredWidth: parent.width
                        Layout.topMargin: 5
                        visible: false
                        Label {
                            id: blockedQuoteFrameLabel
                            font.pointSize: AdjustedValues.f10
                            text: (postFrame.quoteRecordStatus === TimelineListModel.QuoteRecordDetached
                                   ? qsTr("detached by author") : qsTr("blocked"))
                        }
                    }


                    PostInformation {
                        id: postInformation
                        Layout.preferredWidth: parent.width
                    }

                    PostControls {
                        id: postControls
                        Layout.preferredWidth: parent.width
                    }
                }
            }
        }
    }
}
