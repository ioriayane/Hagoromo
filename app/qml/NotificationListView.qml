import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.notificationlistmodel 1.0

import "parts"

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
            icon.source: "images/expand_less.png"
            onClicked: listNotificationListModel.getLatest()
        }
        footer: Button {
            width: rootListView.width
            height: 24
            display: AbstractButton.IconOnly
            icon.source: "images/expand_more.png"
        }

        delegate: Frame {
            id: postFrame
            padding: 10

            property int basisWidth: rootListView.width - postFrame.padding * 2

            Author {
                Layout.maximumWidth: postFrame.basisWidth
                displayName: model.displayName
                handle: ""
                indexedAt: ""
            }

        }
    }
}
