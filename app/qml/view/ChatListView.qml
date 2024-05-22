import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"

ScrollView {
    id: chatListView
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    clip: true

    property alias listView: rootListView
    property alias model: rootListView.model

    signal requestViewChatMessage(string convo_id)

    ListView {
        id: rootListView
        anchors.fill: parent
        anchors.rightMargin: parent.ScrollBar.vertical.width
        spacing: 5
        maximumFlickVelocity: AdjustedValues.maximumFlickVelocity

        onMovementEnded: {
            if(atYEnd){
                rootListView.model.getNext()
            }
        }

        header: ItemDelegate {
            width: rootListView.width
            height: AdjustedValues.h24
            display: AbstractButton.IconOnly
            icon.source: rootListView.model.running ? "" : "../images/expand_less.png"
            onClicked: rootListView.model.getLatest()

            BusyIndicator {
                anchors.centerIn: parent
                width: AdjustedValues.i24
                height: AdjustedValues.i24
                visible: rootListView.model.running
            }
        }
        footer: BusyIndicator {
            width: rootListView.width
            height: AdjustedValues.i24
            visible: rootListView.model.running && rootListView.model.rowCount() > 0
        }

        delegate: ClickableFrame {
            id: chatItemLayout
            clip: true
            topPadding: 10
            leftPadding: 10
            rightPadding: 10
            bottomPadding: 10

            property int layoutWidth: rootListView.width

            onClicked: chatListView.requestViewChatMessage(model.id)

            RowLayout {
                AvatarImage {
                    id: postAvatarImage
                    Layout.preferredWidth: AdjustedValues.i36
                    Layout.preferredHeight: AdjustedValues.i36
                    source: model.memberAvatars.length > 0 ? model.memberAvatars[0] : "../images/account_icon.png"
                }
                ColumnLayout {
                    property int basisWidth: chatItemLayout.layoutWidth - chatItemLayout.leftPadding - chatItemLayout.rightPadding -
                                             postAvatarImage.width - parent.spacing

                    RowLayout {
                        Layout.preferredWidth: parent.basisWidth
                        Label {
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                            font.pointSize: AdjustedValues.f10
                            text: model.memberDisplayNames.length > 0 ? model.memberDisplayNames[0] : ""
                        }
                        Label {
                            Layout.preferredWidth: implicitWidth
                            Layout.minimumWidth: implicitWidth
                            font.pointSize: AdjustedValues.f8
                            color: Material.color(Material.Grey)
                            text: model.lastMessageSentAt
                        }
                    }

                    Label {
                        Layout.preferredWidth: parent.basisWidth
                        font.pointSize: AdjustedValues.f10
                        color: Material.color(Material.Grey)
                        text: model.lastMessageText
                        textFormat: Text.StyledText
                        wrapMode: Text.WrapAnywhere

                        Rectangle {
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            width: childrenRect.width * 2
                            height: childrenRect.height
                            radius: height / 2
                            visible: model.unreadCountRole > 0
                            color: Material.accentColor
                            Label {
                                id: unreadCountLabel
                                x: width / 2
                                font.pointSize: AdjustedValues.f8
                                text: model.unreadCountRole
                            }
                        }
                    }
                }
            }
        }
    }
}
