import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.notificationlistmodel 1.0

import "../parts"

ScrollView {
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    property alias model: rootListView.model
    ListView {
        id: rootListView
        anchors.fill: parent
        anchors.rightMargin: parent.ScrollBar.vertical.width

        model: NotificationListModel {
            id: listNotificationListModel
        }

        header: Button {
            width: rootListView.width
            height: 24
            display: AbstractButton.IconOnly
            icon.source: "../images/expand_less.png"
            onClicked: listNotificationListModel.getLatest()
        }
        footer: Button {
            width: rootListView.width
            height: 24
            display: AbstractButton.IconOnly
            icon.source: "../images/expand_more.png"
        }

        delegate: Frame {
            id: postFrame
            width: rootListView.width
            padding: 10

            property int basisWidth: rootListView.width - postFrame.padding * 2 - postLayout.spacing - reasonImage.width

            RowLayout {
                id: postLayout
                Image {
                    id: reasonImage
                    Layout.preferredWidth: 16
                    Layout.preferredHeight: 16
                    Layout.alignment: Qt.AlignTop
                    source: {
                        if(model.reason === "like"){
                            return "../images/like.png"
                        }else if(model.reason === "repost"){
                            return "../images/repost.png"
                        }else if(model.reason === "follow"){
                            return "../images/add_user.png"
                        }else if(model.reason === "mention"){
                            return "../images/reply.png"
                        }else if(model.reason === "reply"){
                            return "../images/reply.png"
                        }else if(model.reason === "quote"){
                            return "../images/reply.png"
                        }else{
                            return "../images/like.png"
                        }
                    }
                    layer.enabled: true
                    layer.effect: ColorOverlay {
                        color: {
                            if(model.reason === "like"){
                                return Material.color(Material.Pink)
                            }else if(model.reason === "repost"){
                                return Material.color(Material.Green)
                            }else if(model.reason === "follow"){
                                return Material.color(Material.LightBlue)
                            }else if(model.reason === "mention"){
                                return Material.color(Material.Blue)
                            }else if(model.reason === "reply"){
                                return Material.color(Material.BlueGrey)
                            }else if(model.reason === "quote"){
                                return Material.color(Material.Lime)
                            }else{
                                return Material.color(Material.Grey)
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    RowLayout {
                        Image {
                            Layout.preferredWidth: 16
                            Layout.preferredHeight: 16
                            source: model.avatar
                        }
                        Author {
                            Layout.fillWidth: true
                            displayName: model.displayName
                            handle: model.handle
                            indexedAt: model.indexedAt
                        }
                    }

//                    Label {
//                        text: "test"
//                    }
                }
            }
        }
    }
}
