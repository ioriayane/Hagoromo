import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

Frame {
    id: postFrame
    //    width: rootListView.width
    topPadding: 10
    leftPadding: 10
    rightPadding: 10
    bottomPadding: 2

    property alias repostReactionAuthor: repostReactionAuthor
    property alias replyReactionAuthor: replyReactionAuthor
    property alias postAvatarImage: postAvatarImage
    property alias postAuthor: postAuthor
    property alias recordText: recordText
    property alias recordTextMouseArea: recordTextMouseArea
    property alias postImagePreview: postImagePreview
    property alias childFrame: childFrame
    property alias childAvatarImage: childAvatarImage
    property alias childAuthor: childAuthor
    property alias childRecordText: childRecordText
    property alias postControls: postControls

    ColumnLayout {
        ReactionAuthor {
            id: repostReactionAuthor
            source: "../images/repost.png"
            color: Material.color(Material.Green)
        }
        ReactionAuthor {
            id: replyReactionAuthor
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
                    // wrapMode: Text.Wrap
                    wrapMode: Text.WrapAnywhere
                    font.pointSize: 10
                    //                    font.letterSpacing: 0
                    lineHeight: 1.3
                    //                            font.family: "遊ゴシック"  // "メイリオ"  "BIZ UDPゴシック"

                    MouseArea {
                        id: recordTextMouseArea
                        anchors.fill: parent
                    }
                }
                ImagePreview {
                    id: postImagePreview
                    layoutWidth: recordText.width
                    Layout.topMargin: 5
                }

                Frame {
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
                                wrapMode: Text.WrapAnywhere
                                font.pointSize: 10
                                lineHeight: 1.3
                            }
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
