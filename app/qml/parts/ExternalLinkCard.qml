import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "../controls"

ClickableFrame {
    id: externalLinkFrame
    contentWidth: contentRootLayout.implicitWidth
    contentHeight: contentRootLayout.implicitHeight

    topInset: 0
    leftInset: 0
    rightInset: 0
    bottomInset: 0
    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 5

    property alias thumbImage: thumbImage
    property alias titleLabel: titleLabel
    property alias uriLabel: uriLabel
    property alias descriptionLabel: descriptionLabel

    ColumnLayout {
        id: contentRootLayout
        spacing: 3
        ImageWithIndicator {
            id: thumbImage
            Layout.preferredWidth: externalLinkFrame.width
            Layout.preferredHeight: thumbImage.source !== "" ? externalLinkFrame.width * 0.5 : 5
            fillMode: Image.PreserveAspectCrop
        }
        Label {
            id: titleLabel
            Layout.preferredWidth: externalLinkFrame.width
            leftPadding: 5
            rightPadding: 5
            elide: Label.ElideRight
            font.pointSize: AdjustedValues.f10
        }
        Label {
            id: uriLabel
            Layout.preferredWidth: externalLinkFrame.width
            leftPadding: 5
            rightPadding: 5
            elide: Label.ElideRight
            font.pointSize: AdjustedValues.f8
            color: Material.color(Material.Grey)
        }
        Label {
            id: descriptionLabel
            visible: text.length > 0
            Layout.preferredWidth: externalLinkFrame.width
            leftPadding: 5
            rightPadding: 5
            elide: Label.ElideRight
            font.pointSize: AdjustedValues.f8
        }
    }
}
