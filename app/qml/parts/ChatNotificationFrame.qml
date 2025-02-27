import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import tech.relog.hagoromo.chatnotificationlistmodel
import tech.relog.hagoromo.singleton 1.0

ColumnLayout {
    signal requestAddChatColumn(string uuid)

    Repeater {
        // model: ChatNotificationListModel {
        // }
        model: ListModel {
            ListElement {
                property string accountUuid: "4d8578fd-ce13-4ee1-90eb-c50a9248777a"
                property string avator: "https://cdn.bsky.app/img/avatar/plain/did:plc:ipj5qejfoqu6eukvt72uhyit/bafkreifjldy2fbgjfli7dson343u2bepzwypt7vlffb45ipsll6bjklphy@jpeg"
                property int unreadCount: 1
            }
            ListElement {
                property string accountUuid: "4d8578fd-ce13-4ee1-90eb-c50a9248777a"
                property string avator: "https://cdn.bsky.app/img/avatar/plain/did:plc:mqxsuw5b5rhpwo4lw6iwlid5/bafkreiaeoiy6fqjypbhbcrb3jdlnjtpnwri5wa6jrvbwxtbtey6synwxr4@jpeg"
                property int unreadCount: 3
            }
            ListElement {
                property string accountUuid: "4d8578fd-ce13-4ee1-90eb-c50a9248777a"
                property string avator: "https://cdn.bsky.app/img/avatar/plain/did:plc:73l5atmh7p3fn3xigbp6ao5x/bafkreif42yycinokltjrfizxgvyyw4z63a264jc4ws723pauxtc7vqo7la@jpeg"
                property int unreadCount: 2
            }
        }

        delegate: Frame {
            background: Rectangle {
                radius: 3
                border.width: 1
                border.color: Material.frameColor
                color: Material.backgroundColor
            }
            contentItem: MouseArea {
                onClicked: (mouse) => requestAddChatColumn(model.accountUuid)
            }
            RowLayout {
                AvatarImage {
                    Layout.preferredWidth: AdjustedValues.i24
                    Layout.preferredHeight: AdjustedValues.i24
                    source: model.avator
                }
                Label {
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("You've got message(s)")
                }
                Rectangle {
                    width: childrenRect.width * 2
                    height: childrenRect.height
                    radius: height / 2
                    visible: model.unreadCount > 0
                    color: Material.accentColor
                    Label {
                        id: unreadCountLabel
                        x: width / 2
                        font.pointSize: AdjustedValues.f8
                        text: model.unreadCount
                    }
                }
            }
        }
    }
}
