import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.singleton 1.0

import "../controls"

ClickableFrame {
    property alias avatarImage: avatarImage
    property alias displayNameLabel: displayNameLabel
    property alias creatorHandleLabel: creatorHandleLabel
    property alias descriptionLabel: descriptionLabel

    ColumnLayout {
        GridLayout {
            columns: 2
            rowSpacing: 3
            AvatarImage {
                id: avatarImage
                Layout.preferredWidth: AdjustedValues.i24
                Layout.preferredHeight: AdjustedValues.i24
                Layout.rowSpan: 2
                altSource: "../images/account_icon.png"
            }
            Label {
                id: displayNameLabel
                Layout.fillWidth: true
                font.pointSize: AdjustedValues.f10
            }
            Label {
                id: creatorHandleLabel
                color: Material.color(Material.Grey)
                font.pointSize: AdjustedValues.f8
            }
        }
        Label {
            id: descriptionLabel
            Layout.leftMargin: 3
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true
            font.pointSize: AdjustedValues.f8
        }
    }
}
