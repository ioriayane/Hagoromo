import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.singleton 1.0

import "../controls"

ClickableFrame {
    property alias avatarImage: feedGeneratorAvatarImage
    property alias displayNameLabel: feedGeneratorDisplayNameLabel
    property alias creatorHandleLabel: feedGeneratorCreatorHandleLabel
    property alias likeCountLabel: feedGeneratorLikeCountLabel

    ColumnLayout {
        GridLayout {
            columns: 2
            rowSpacing: 3
            AvatarImage {
                id: feedGeneratorAvatarImage
                Layout.preferredWidth: AdjustedValues.i24
                Layout.preferredHeight: AdjustedValues.i24
                Layout.rowSpan: 2
                altSource: "../images/account_icon.png"
            }
            Label {
                id: feedGeneratorDisplayNameLabel
                Layout.fillWidth: true
                font.pointSize: AdjustedValues.f10
            }
            Label {
                id: feedGeneratorCreatorHandleLabel
                color: Material.color(Material.Grey)
                font.pointSize: AdjustedValues.f8
            }
        }
        RowLayout {
            Layout.leftMargin: 3
            spacing: 3
            Image {
                Layout.preferredWidth: AdjustedValues.i16
                Layout.preferredHeight: AdjustedValues.i16
                source: "../images/like.png"
                layer.enabled: true
                layer.effect: ColorOverlay {
                    color: Material.color(Material.Pink)
                }
            }
            Label {
                id: feedGeneratorLikeCountLabel
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                font.pointSize: AdjustedValues.f8
            }
        }
    }
}
