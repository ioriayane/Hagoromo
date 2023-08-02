import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import "../controls"

ColumnLayout {
    anchors.fill: parent
    anchors.margins: 1
    spacing: 0

    property alias post: post
    property alias search: search
    property alias addColumn: addColumn
    property alias moderation: moderation
    property alias account: account
    property alias setting: setting

    IconButton {
        id: post
        Layout.fillWidth: true
        Layout.preferredHeight: 36
        enabled: accountListModel.count > 0
        display: AbstractButton.IconOnly
        iconSource: "../images/edit.png"
        //                    iconText: qsTr("New Post")
    }

    IconButton {
        id: search
        Layout.fillWidth: true
        Layout.preferredHeight: 36
        enabled: accountListModel.count > 0
        display: AbstractButton.IconOnly
        iconSource: "../images/search.png"
    }

    Item {
        Layout.preferredWidth: 1
        Layout.fillHeight: true
    }

    IconButton {
        id: addColumn
        Layout.fillWidth: true
        Layout.preferredHeight: 36
        enabled: accountListModel.count > 0
        display: AbstractButton.IconOnly
        iconSource: "../images/column.png"
        //                    iconText: qsTr("Add column")
    }

    IconButton {
        id: moderation
        Layout.fillWidth: true
        Layout.preferredHeight: 36
        visible: false
        enabled: accountListModel.count > 0
        display: AbstractButton.IconOnly
        iconSource: "../images/hand.png"
    }

    IconButton {
        id: account
        Layout.fillWidth: true
        Layout.preferredHeight: 36
        display: AbstractButton.IconOnly
        iconSource: "../images/account.png"
        //                    iconText: qsTr("Account")
    }

    IconButton {
        id: setting
        Layout.fillWidth: true
        Layout.preferredHeight: 36
        display: AbstractButton.IconOnly
        iconSource: "../images/settings.png"
        //                    iconText: qsTr("Settings")
    }
}
