import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.draftlistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"

Dialog {
    id: selectDraftDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Draft")

    property alias account: account
    Account {
        id: account
    }

    onOpened: {
        console.log("SelectDraftDialog : " + account.uuid)
        draftListModel.clear()
        draftListModel.setAccount(account.uuid)
        draftListModel.getLatest()
    }

    ColumnLayout {
        ListView {
            id: draftListView
            Layout.preferredWidth: 500 * AdjustedValues.ratio
            Layout.preferredHeight: 300 * AdjustedValues.ratio
            maximumFlickVelocity: AdjustedValues.maximumFlickVelocity
            clip: true
            model: DraftListModel {
                id: draftListModel
            }

            delegate: ClickableFrame {
                id: contentRootFrame
                contentWidth: contentRootLayout.implicitWidth
                contentHeight: contentRootLayout.implicitHeight
                topPadding: 10
                leftPadding: 10
                rightPadding: 10
                bottomPadding: 2

                ColumnLayout {
                    id: contentRootLayout
                    implicitWidth: basisWidth
                    property real basisWidth: draftListView.width - contentRootFrame.leftPadding - contentRootFrame.rightPadding

                    Label {
                        Layout.preferredWidth: parent.width
                        textFormat: Text.StyledText
                        wrapMode: Text.Wrap
                        font.pointSize: AdjustedValues.f10
                        lineHeight: 1.3
                        text: model.primaryText
                    }
                    Label {
                        Layout.topMargin: 5
                        font.pointSize: AdjustedValues.f8
                        color: Material.color(Material.Grey)
                        text: model.updatedAt
                    }
                }
            }
        }

        RowLayout {
            Button {
                flat: true
                font.pointSize: AdjustedValues.f10
                text: qsTr("Cancel")
                onClicked: {
                    selectDraftDialog.reject()
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Apply")
                // enabled: selectThreadGateDialog.ready
                BusyIndicator {
                    anchors.fill: parent
                    anchors.margins: 3
                    visible: !parent.enabled
                }
                onClicked: {

                    selectDraftDialog.accept()
                }
            }
        }

    }
}
