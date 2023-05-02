import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ListView {
    id: timelineListView


    header: Button {
        width: timelineListView.width
        height: 24
        display: AbstractButton.IconOnly
        icon.source: "images/expand_less.png"
    }
    footer: Button {
        width: timelineListView.width
        height: 24
        display: AbstractButton.IconOnly
        icon.source: "images/expand_more.png"
    }

    delegate: Rectangle{
        border.width: 1
        border.color: "#aaaaaa"
        width: timelineListView.width
        height: postLayout.height
        RowLayout {
            id: postLayout
            spacing: 0
            Image {
                Layout.preferredWidth: 48
                Layout.preferredHeight: 48
                Layout.alignment: Qt.AlignTop
                Layout.margins: 5
                source: model.avatar
            }
            ColumnLayout {
                id: bodyLayout
                Layout.fillWidth: true
                Layout.margins: 5
                spacing: 5
                RowLayout {
                    visible: model.hasParent
                    Image {
                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 16
                        source: "images/reply.png"
                    }
                    Text {
                        Layout.alignment: Qt.AlignCenter
                        text: model.parentDisplayName
                        font.pointSize: 10
                    }
                }

                RowLayout {
                    id: headerLayout
                    Layout.maximumWidth: parent.width
                    Text {
                        Layout.maximumWidth: headerLayout.width - handleText.width - indexAtText.width - headerLayout.spacing * 3
                        font.pointSize: 12
                        elide: Text.ElideRight
                        text: model.displayName
                    }
                    Text {
                        id: handleText
                        opacity: 0.8
                        font.pointSize: 10
                        text: model.handle
                    }
                    Item {
                        id: spacerLine1Item
                        Layout.fillWidth: true
                    }
                    Text {
                        id: indexAtText
                        opacity: 0.8
                        font.pointSize: 10
                        text: model.indexedAt
                    }
                }
                Text {
                    id: recordText
                    Layout.preferredWidth: timelineListView.width - 20 - 48
                    wrapMode: Text.Wrap// Text.WrapAnywhere
                    font.pointSize: 12
                    text: model.recordText
//                    Rectangle {
//                        anchors.fill: parent
//                        color: "#22ff0000"
//                    }
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
