import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import tech.relog.hagoromo.chatnotificationlistmodel
import tech.relog.hagoromo.singleton 1.0

ColumnLayout {
    id: chatNotificationFrame

    signal requestAddChatColumn(string uuid)

    Repeater {
        model: ChatNotificationListModel {
            id: chatNotificationListModel
            enabled: chatNotificationFrame.enabled
        }

        delegate: Frame {
            visible: model.visible

            background: Rectangle {
                radius: 3
                border.width: 1
                border.color: Material.frameColor
                color: Material.backgroundColor
            }
            contentItem: MouseArea {
                onClicked: (mouse) => {
                               chatNotificationListModel.hideItem(model.index)
                               requestAddChatColumn(model.accountUuid)
                           }
            }
            RowLayout {
                AvatarImage {
                    Layout.preferredWidth: AdjustedValues.i24
                    Layout.preferredHeight: AdjustedValues.i24
                    source: model.avatar
                }
                Label {
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("You've got message(s).")
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
