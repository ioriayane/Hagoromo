import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import "../controls"

ClickableFrame {
    id: postFrame
    //    width: rootListView.width
    topPadding: 10
    leftPadding: 10
    rightPadding: 10
    bottomPadding: 2
    style: "Post"

    property alias repostReactionAuthor: repostReactionAuthor
    property alias replyReactionAuthor: replyReactionAuthor
    property alias postAvatarImage: postAvatarImage
    property alias postAuthor: postAuthor
    property alias recordText: recordText
    property alias postImagePreview: postImagePreview
    property alias childFrame: childFrame
    property alias childAvatarImage: childAvatarImage
    property alias childAuthor: childAuthor
    property alias childRecordText: childRecordText
    property alias externalLinkFrame: externalLinkFrame
    property alias externalLinkThumbImage: externalLinkThumbImage
    property alias externalLinkTitleLabel: externalLinkTitleLabel
    property alias externalLinkUriLabel: externalLinkUriLabel
    property alias externalLinkDescriptionLabel: externalLinkDescriptionLabel
    property alias postControls: postControls


    ColumnLayout {
        ReactionAuthor {
            id: repostReactionAuthor
            Layout.maximumWidth: postFrame.contentWidth
            source: "../images/repost.png"
            color: Material.color(Material.Green)
        }
        ReactionAuthor {
            id: replyReactionAuthor
            Layout.maximumWidth: postFrame.contentWidth
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
                Layout.fillWidth: true
                spacing: 0

                property int basisWidth: postFrame.width - postFrame.leftPadding - postFrame.rightPadding -
                                         postLayout.spacing - postAvatarImage.Layout.preferredWidth

                Author {
                    id: postAuthor
                    Layout.maximumWidth: parent.basisWidth
                }

                Label {
                    id: recordText
                    Layout.preferredWidth: parent.basisWidth
                    Layout.maximumWidth: parent.basisWidth
                    Layout.topMargin: 5
                    textFormat: Text.StyledText
                    wrapMode: Text.WrapAnywhere
                    font.pointSize: 10
                    lineHeight: 1.3
                    onLinkActivated: (url) => Qt.openUrlExternally(url)
                }
                ImagePreview {
                    id: postImagePreview
                    layoutWidth: recordText.width
                    Layout.topMargin: 5
                }

                ClickableFrame {
                    id: childFrame
                    Layout.fillWidth: true
                    Layout.topMargin: 5
                    RowLayout {
                        id: childLayout
                        spacing: 10
                        AvatarImage {
                            id: childAvatarImage
                            Layout.preferredWidth: 16
                            Layout.preferredHeight: 16
                            Layout.alignment: Qt.AlignTop
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            property int basisWidth: recordText.width - childFrame.padding * 2 - childLayout.spacing - childAvatarImage.Layout.preferredWidth
                            Author {
                                id: childAuthor
                                Layout.maximumWidth: parent.basisWidth
                            }
                            Label {
                                id: childRecordText
                                Layout.preferredWidth: parent.basisWidth
                                Layout.maximumWidth: parent.basisWidth
                                textFormat: Text.StyledText
                                wrapMode: Text.WrapAnywhere
                                font.pointSize: 10
                                lineHeight: 1.3
                                onLinkActivated: (url) => Qt.openUrlExternally(url)
                            }
                        }
                    }
                }

                ClickableFrame {
                    id: externalLinkFrame
                    Layout.preferredWidth: recordText.width
                    Layout.topMargin: 5
                    visible: false
                    topInset: 0
                    leftInset: 0
                    rightInset: 0
                    bottomInset: 0
                    topPadding: 0
                    leftPadding: 0
                    rightPadding: 0
                    bottomPadding: 5
                    ColumnLayout {
                        spacing: 3
                        ImageWithIndicator {
                            id: externalLinkThumbImage
                            Layout.preferredWidth: externalLinkFrame.width
                            Layout.preferredHeight: externalLinkFrame.width * 0.5
                            fillMode: Image.PreserveAspectCrop
                        }
                        Label {
                            id: externalLinkTitleLabel
                            Layout.preferredWidth: externalLinkFrame.width
                            leftPadding: 5
                            rightPadding: 5
                            elide: Label.ElideRight
                        }
                        Label {
                            id: externalLinkUriLabel
                            Layout.preferredWidth: externalLinkFrame.width
                            leftPadding: 5
                            rightPadding: 5
                            elide: Label.ElideRight
                            font.pointSize: 8
                            color: Material.color(Material.Grey)
                        }
                        Label {
                            id: externalLinkDescriptionLabel
                            visible: text.length > 0
                            Layout.preferredWidth: externalLinkFrame.width
                            leftPadding: 5
                            rightPadding: 5
                            elide: Label.ElideRight
                            font.pointSize: 8
                        }
                    }
                }

                PostControls {
                    id: postControls
                }
            }
        }
    }
}
