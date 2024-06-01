import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.feedgeneratorlistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"
import "../view"

Dialog {
    id: discoverFeedsDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Discover Feeds")

    property alias account: account
    Account {
        id: account
    }
    property string selectedName: ""
    property string selectedUri: ""
    signal errorOccured(string account_uuid, string code, string message)

    onOpened: {
        if(account.service.length === 0){
            return
        }

        feedGeneratorListModel.setAccount(account.service, account.did, account.handle,
                                          account.email, account.accessJwt, account.refreshJwt)
        feedGeneratorListModel.getLatest()

        generatorScrollView.currentIndex = -1
    }
    onClosed: {
        searchText.text = ""
    }

    Shortcut {  // Search
        enabled: discoverFeedsDialog.visible && searchButton.enabled && searchText.focus
        sequence: "Ctrl+Return"
        onActivated: searchButton.clicked()
    }

    ColumnLayout {
        spacing: 10
        RowLayout {
            AvatarImage {
                Layout.preferredWidth: AdjustedValues.i24
                Layout.preferredHeight: AdjustedValues.i24
                source: account.avatar
            }
            Label {
                font.pointSize: AdjustedValues.f10
                text: account.handle
                elide: Text.ElideRight
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
        }

        RowLayout {
            spacing: AdjustedValues.s5
            TextField  {
                id: searchText
                Layout.fillWidth: true
                selectByMouse: true
                font.pointSize: AdjustedValues.f10
            }
            Button {
                id: searchButton
                enabled: searchText.text.length > 0 && !feedGeneratorListModel.running
                font.pointSize: AdjustedValues.f10
                text: qsTr("Search")
                onClicked: {
                    discoverFeedsDialog.selectedName = ""
                    discoverFeedsDialog.selectedUri = ""
                    feedGeneratorListModel.getLatest()
                }
                BusyIndicator {
                    anchors.fill: parent
                    anchors.margins: 3
                    visible: feedGeneratorListModel.running
                }
            }
        }
        FeedGeneratorListView {
            id: generatorScrollView
            Layout.preferredWidth: 400 * AdjustedValues.ratio
            Layout.preferredHeight: 350 * AdjustedValues.ratioHalf
            model: FeedGeneratorListModel {
                id: feedGeneratorListModel
                query: searchText.text
                onErrorOccured: (code, message) => discoverFeedsDialog.errorOccured(discoverFeedsDialog.account.uuid, code, message)
            }
            onClicked: (display_name, uri) => {
                           discoverFeedsDialog.selectedName = display_name
                           discoverFeedsDialog.selectedUri = uri
                       }
            onRequestRemoveGenerator: (uri) => feedGeneratorListModel.removeGenerator(uri)
            onRequestSaveGenerator: (uri) => feedGeneratorListModel.saveGenerator(uri)
        }
        RowLayout {
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Cancel")
                flat: true
                onClicked: discoverFeedsDialog.reject()
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                id: addButton
                enabled: generatorScrollView.currentIndex >= 0 && discoverFeedsDialog.selectedUri.length > 0
                font.pointSize: AdjustedValues.f10
                text: qsTr("Add")
                onClicked: {
                    discoverFeedsDialog.accept()
                }
            }
        }
    }
}
