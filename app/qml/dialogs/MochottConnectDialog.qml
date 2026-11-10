import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

// mochottとの連携の前に規約などへの同意を得る
Dialog {
    id: mochottConnectDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    closePolicy: Dialog.CloseOnEscape

    bottomPadding: AdjustedValues.s5

    title: qsTr("Post a long post with mochott")

    ColumnLayout {
        spacing: AdjustedValues.s10
        Label {
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            wrapMode: Text.Wrap
            font.pointSize: AdjustedValues.f10
            text: qsTr("Posts over 300 characters are posted as a long post with mochott. " +
                       "The full text is saved as a mochott article in your repository, " +
                       "and the beginning is posted to Bluesky.") + "\n\n" +
                  qsTr("To continue, please agree to the following and link your account with mochott in your browser. " +
                       "After linking, press Long post again.")
        }
        Label {
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            wrapMode: Text.Wrap
            font.pointSize: AdjustedValues.f10
            textFormat: Text.StyledText
            text: "<a href=\"https://mochott.site/terms\">" + qsTr("Terms of Service") + "</a><br/>" +
                  "<a href=\"https://mochott.site/privacy\">" + qsTr("Privacy Policy") + "</a><br/>" +
                  "<a href=\"https://mochott.site/guideline\">" + qsTr("Community Guidelines") + "</a>"
            onLinkActivated: (url) => Qt.openUrlExternally(url)
            HoverHandler {
                cursorShape: parent.hoveredLink.length > 0 ? Qt.PointingHandCursor : Qt.ArrowCursor
            }
        }
        RowLayout {
            Layout.alignment: Qt.AlignRight
            Button {
                font.pointSize: AdjustedValues.f10
                flat: true
                text: qsTr("Cancel")
                onClicked: mochottConnectDialog.reject()
            }
            Button {
                font.pointSize: AdjustedValues.f10
                font.bold: true
                text: qsTr("Agree and link")
                onClicked: mochottConnectDialog.accept()
            }
        }
    }
}
