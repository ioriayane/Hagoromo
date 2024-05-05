import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.logoperator 1.0
import tech.relog.hagoromo.logstatisticslistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../data"
import "../parts"
import "../view"

Dialog {
    id: logViewDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Log")

    signal errorOccured(string account_uuid, string code, string message)

    property alias account: account

    onOpened: {
        logStatisticsListModel.getLatest()
    }
    onClosed: {
        logStatisticsListModel.clear()
    }


    Account {
        id: account
    }
    LogOperator {
        id: logOperator
        service: account.service
        did: account.did
        onFinished: {
            logStatisticsListModel.getLatest()
        }
        onErrorOccured: (code, message) => logViewDialog.errorOccured(account.uuid, code, message)
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
                Layout.fillWidth: true
                font.pointSize: AdjustedValues.f10
                text: account.handle
                elide: Text.ElideRight
            }
            IconButton {
                Layout.preferredWidth: AdjustedValues.b30
                Layout.preferredHeight: AdjustedValues.b24
                iconSource: "../images/refresh.png"
                iconSize: AdjustedValues.i16
                enabled: !logOperator.running
                onClicked: {
                    logOperator.getLatest()
                }
                BusyIndicator {
                    anchors.fill: parent
                    visible: logOperator.running
                }
            }
        }
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            TabButton {
                font.pointSize: AdjustedValues.f10
                font.capitalization: Font.MixedCase
                text: qsTr("Statistics")
            }
            TabButton {
                font.pointSize: AdjustedValues.f10
                font.capitalization: Font.MixedCase
                text: qsTr("Daily")
            }
            TabButton {
                font.pointSize: AdjustedValues.f10
                font.capitalization: Font.MixedCase
                text: qsTr("Monthly")
            }
        }
        SwipeView {
            currentIndex: tabBar.currentIndex
            interactive: false
            clip: true

            Frame {
                contentWidth: statisticsScrollView.width
                contentHeight: statisticsScrollView.height

                LogTotalListView {
                    id: statisticsScrollView
                    width: 500
                    height: 350
                    verticalScrollBar: false
                    model: LogStatisticsListModel {
                        id: logStatisticsListModel
                        did: account.did
                    }
                }
            }
            Frame {

            }
            Frame {

            }
        }
        RowLayout {
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Close")
                flat: true
                onClicked: logViewDialog.close()
            }
            Item {
                Layout.fillWidth: true
            }
        }
    }
}
