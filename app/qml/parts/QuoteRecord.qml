import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "../controls"

ClickableFrame {
    id: quoteRecordFrame
    contentWidth: quoteRecordLayout.implicitWidth
    contentHeight: quoteRecordLayout.implicitHeight

    property int basisWidth: width
    property alias quoteRecordAvatarImage: quoteRecordAvatarImage
    property alias quoteRecordAuthor: quoteRecordAuthor
    property alias quoteRecordRecordText: quoteRecordRecordText
    property alias quoteRecordImagePreview: quoteRecordImagePreview
    property alias quoteRecordEmbedVideoFrame: quoteRecordEmbedVideoFrame

    signal openLink(string url)
    signal displayLink(string url)

    RowLayout {
        id: quoteRecordLayout
        spacing: 10
        AvatarImage {
            id: quoteRecordAvatarImage
            Layout.preferredWidth: AdjustedValues.i16
            Layout.preferredHeight: AdjustedValues.i16
            Layout.alignment: Qt.AlignTop
        }
        ColumnLayout {
            Layout.fillWidth: true
            property int basisWidth: quoteRecordFrame.basisWidth - quoteRecordFrame.padding * 2 -
                                     quoteRecordLayout.spacing - quoteRecordAvatarImage.Layout.preferredWidth
            Author {
                id: quoteRecordAuthor
                layoutWidth: parent.basisWidth
            }
            Label {
                id: quoteRecordRecordText
                Layout.preferredWidth: parent.basisWidth
                Layout.maximumWidth: parent.basisWidth
                visible: text.length > 0
                textFormat: Text.StyledText
                wrapMode: Text.Wrap
                font.pointSize: AdjustedValues.f10
                lineHeight: 1.3
                onLinkActivated: (url) => openLink(url)
                onHoveredLinkChanged: displayLink(hoveredLink)
            }
            ImagePreview {
                id: quoteRecordImagePreview
                layoutWidth: parent.basisWidth
                Layout.topMargin: 5
            }
            VideoFrame {
                id: quoteRecordEmbedVideoFrame
                Layout.preferredWidth: parent.width
                Layout.topMargin: 5
            }
        }
    }
}
