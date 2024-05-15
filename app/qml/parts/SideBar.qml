import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import tech.relog.hagoromo.singleton 1.0

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
        width: parent.width
        Layout.preferredWidth: parent.width
        Layout.preferredHeight: AdjustedValues.b36
        enabled: accountListModel.count > 0
        display: AbstractButton.IconOnly
        iconSource: "../images/edit.png"
        //                    iconText: qsTr("New Post")
        onWidthChanged: console.log("post:" + width)
    }

    IconButton {
        id: search
        width: parent.width
        Layout.preferredWidth: parent.width
        Layout.preferredHeight: AdjustedValues.b36
        enabled: accountListModel.count > 0
        display: AbstractButton.IconOnly
        iconSource: "../images/search.png"
        onWidthChanged: console.log("search:" + width)
    }

    Item {
        Layout.preferredWidth: 1
        Layout.fillHeight: true
    }

    IconButton {
        id: addColumn
        width: parent.width
        Layout.preferredWidth: parent.width
        Layout.preferredHeight: AdjustedValues.b36
        enabled: accountListModel.count > 0
        display: AbstractButton.IconOnly
        iconSource: "../images/column.png"
        //                    iconText: qsTr("Add column")
    }

    IconButton {
        id: moderation
        width: parent.width
        Layout.preferredWidth: parent.width
        Layout.preferredHeight: AdjustedValues.b36
        visible: false
        enabled: accountListModel.count > 0
        display: AbstractButton.IconOnly
        iconSource: "../images/hand.png"
    }

    IconButton {
        id: account
        width: parent.width
        Layout.preferredWidth: parent.width
        Layout.preferredHeight: AdjustedValues.b36
        display: AbstractButton.IconOnly
        iconSource: "../images/account.png"
        //                    iconText: qsTr("Account")
    }

    IconButton {
        id: setting
        width: parent.width
        Layout.preferredWidth: parent.width
        Layout.preferredHeight: AdjustedValues.b36
        display: AbstractButton.IconOnly
        iconSource: "../images/settings.png"
        //                    iconText: qsTr("Settings")
    }
}
