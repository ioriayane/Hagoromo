import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.logoperator 1.0
import tech.relog.hagoromo.logstatisticslistmodel 1.0
import tech.relog.hagoromo.logdailylistmodel 1.0
import tech.relog.hagoromo.logmonthlylistmodel 1.0
import tech.relog.hagoromo.logfeedlistmodel 1.0
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
    title: qsTr("Statistics and logs")

    signal errorOccured(string account_uuid, string code, string message)

    signal requestReply(string account_uuid,
                        string cid, string uri,
                        string reply_root_cid, string reply_root_uri,
                        string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestQuote(string account_uuid,
                        string cid, string uri,
                        string avatar, string display_name, string handle, string indexed_at, string text)
    signal requestViewImages(int index, var paths, var alts)
    signal requestAddMutedWord(string account_uuid, string text)
    signal requestUpdateThreadGate(string account_uuid, string uri, string threadgate_uri, string type, var rules, var callback)

    property string hoveredLink: ""
    property int parentHeight: parent.height
    property alias account: account

    onOpened: {
        logStatisticsListModel.getLatest()
        logDailyListModel.getLatest()
        logMonthlyListModel.getLatest()
    }
    onClosed: {
        tabBar.currentIndex = 0
        logStatisticsListModel.clear()
        logDailyListModel.clear()
        logMonthlyListModel.clear()
        logSearchFeedListModel.clear()
        logDailyFeedListModel.clear()
        logMonthlyFeedListModel.clear()
        searchText.clear()
    }


    Account {
        id: account
    }
    LogOperator {
        id: logOperator
        service: account.serviceEndpoint
        did: account.did
        onFinished: {
            logStatisticsListModel.getLatest()
            logDailyListModel.getLatest()
            logMonthlyListModel.getLatest()
        }
        onErrorOccured: (code, message) => logViewDialog.errorOccured(account.uuid, code, message)
    }
    Shortcut {  // Search
        enabled: logViewDialog.visible && searchButton.enabled && searchText.focus && tabBar.currentIndex == 1
        sequence: "Ctrl+Return"
        onActivated: searchButton.clicked()
    }

    ColumnLayout {
        spacing: 0
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
            Label {
                Layout.alignment: Qt.AlignBottom
                font.pointSize: AdjustedValues.f8
                visible: logOperator.running
                text: logOperator.progressMessage
            }
            IconButton {
                Layout.preferredWidth: AdjustedValues.b30
                Layout.preferredHeight: AdjustedValues.b24
                iconSource: "../images/refresh.png"
                iconSize: AdjustedValues.i16
                enabled: !logOperator.running &&
                         !logStatisticsListModel.running &&
                         !logDailyListModel.running &&
                         !logMonthlyListModel.running &&
                         !logDailyFeedListModel.running &&
                         !logMonthlyFeedListModel.running &&
                         !logSearchFeedListModel.running
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
            Layout.topMargin: 5
            Layout.fillWidth: true
            TabButton {
                implicitHeight: AdjustedValues.b36
                font.pointSize: AdjustedValues.f10
                font.capitalization: Font.MixedCase
                text: qsTr("Statistics")
            }
            TabButton {
                implicitHeight: AdjustedValues.b36
                font.pointSize: AdjustedValues.f10
                font.capitalization: Font.MixedCase
                text: qsTr("Search")
            }
            TabButton {
                implicitHeight: AdjustedValues.b36
                font.pointSize: AdjustedValues.f10
                font.capitalization: Font.MixedCase
                text: qsTr("Daily")
            }
            TabButton {
                implicitHeight: AdjustedValues.b36
                font.pointSize: AdjustedValues.f10
                font.capitalization: Font.MixedCase
                text: qsTr("Monthly")
            }
        }
        SwipeView {
            id: swipeView
            currentIndex: tabBar.currentIndex
            interactive: false
            clip: true

            property int frameWidth: 600 * AdjustedValues.ratio
            property int frameHeight: parentHeight - 260 * AdjustedValues.ratioHalf
            property int frameColumnWidth: 150 * AdjustedValues.ratio

            Frame {
                contentWidth: statisticsScrollView.width
                contentHeight: statisticsScrollView.height

                LogTotalListView {
                    id: statisticsScrollView
                    width: swipeView.frameWidth
                    height: swipeView.frameHeight
                    verticalScrollBar: true
                    model: LogStatisticsListModel {
                        id: logStatisticsListModel
                        did: account.did
                    }
                }
            }
            Frame {
                contentWidth: searchRowLayout.width
                contentHeight: searchRowLayout.height

                ColumnLayout {
                    id: searchRowLayout
                    width: swipeView.frameWidth
                    height: swipeView.frameHeight
                    RowLayout {
                        Layout.fillWidth: true
                        TextField  {
                            id: searchText
                            Layout.fillWidth: true
                            selectByMouse: true
                            font.pointSize: AdjustedValues.f10
                        }
                        Button {
                            id: searchButton
                            enabled: searchText.text.length > 0
                            font.pointSize: AdjustedValues.f10
                            text: qsTr("Search")
                            onClicked: {
                                console.log("search:" + searchText.text)
                                logSearchFeedListModel.setAccount(account.uuid)
                                logSearchFeedListModel.selectCondition = searchText.text
                                logSearchFeedListModel.clear()
                                logSearchFeedListModel.getLatest()
                            }
                        }
                    }
                    TimelineView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: LogFeedListModel {
                            id: logSearchFeedListModel
                            targetDid: account.did
                            targetHandle: account.handle
                            targetAvatar: account.avatar
                            feedType: LogFeedListModel.WordsFeedType
                        }
                        accountDid: account.did
                        logMode: true
                        onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                                        logViewDialog.requestReply(account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
                        onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                                        logViewDialog.requestQuote(account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)
                        onRequestViewImages: (index, paths, alts) => logViewDialog.requestViewImages(index, paths, alts)
                        onRequestAddMutedWord: (text) => logViewDialog.requestAddMutedWord(account.uuid, text)
                        onRequestUpdateThreadGate: (uri, threadgate_uri, type, rules, callback) =>
                                                   logViewDialog.requestUpdateThreadGate(account.uuid, uri, threadgate_uri, type, rules, callback)
                        onHoveredLinkChanged: logViewDialog.hoveredLink = hoveredLink
                    }
                }
            }
            Frame {
                contentWidth: dailyRowLayout.width
                contentHeight: dailyRowLayout.height

                RowLayout {
                    id: dailyRowLayout
                    width: swipeView.frameWidth
                    LogTotalListView {
                        Layout.preferredWidth: swipeView.frameColumnWidth
                        Layout.preferredHeight: swipeView.frameHeight
                        verticalScrollBar: true
                        enabled: !logDailyFeedListModel.running &&
                                 !logMonthlyFeedListModel.running &&
                                 !logSearchFeedListModel.running
                        model: LogDailyListModel {
                            id: logDailyListModel
                            did: account.did
                            onFinished: {
                                if(rowCount() === 0){
                                    logOperator.getLatest()
                                }
                            }
                        }
                        onClickedItem: (name) => {
                                           console.log("select:" + name)
                                           logDailyFeedListModel.setAccount(account.uuid)
                                           logDailyFeedListModel.selectCondition = name
                                           logDailyFeedListModel.clear()
                                           logDailyFeedListModel.getLatest()
                                       }
                    }
                    TimelineView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: swipeView.frameHeight
                        model: LogFeedListModel {
                            id: logDailyFeedListModel
                            targetDid: account.did
                            targetHandle: account.handle
                            targetAvatar: account.avatar
                            feedType: LogFeedListModel.DailyFeedType
                        }
                        accountDid: account.did
                        logMode: true
                        onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                                        logViewDialog.requestReply(account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
                        onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                                        logViewDialog.requestQuote(account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)
                        onRequestViewImages: (index, paths, alts) => logViewDialog.requestViewImages(index, paths, alts)
                        onRequestAddMutedWord: (text) => logViewDialog.requestAddMutedWord(account.uuid, text)
                        onRequestUpdateThreadGate: (uri, threadgate_uri, type, rules, callback) =>
                                                   logViewDialog.requestUpdateThreadGate(account.uuid, uri, threadgate_uri, type, rules, callback)
                        onHoveredLinkChanged: logViewDialog.hoveredLink = hoveredLink
                    }
                }
            }
            Frame {
                contentWidth: monthlyRowLayout.width
                contentHeight: monthlyRowLayout.height

                RowLayout {
                    id: monthlyRowLayout
                    width: swipeView.frameWidth
                    LogTotalListView {
                        Layout.preferredWidth: swipeView.frameColumnWidth
                        Layout.preferredHeight: swipeView.frameHeight
                        verticalScrollBar: true
                        enabled: !logDailyFeedListModel.running &&
                                 !logMonthlyFeedListModel.running &&
                                 !logSearchFeedListModel.running
                        model: LogMonthlyListModel {
                            id: logMonthlyListModel
                            did: account.did
                        }
                        onClickedItem: (name) => {
                                           console.log("select:" + name)
                                           logMonthlyFeedListModel.setAccount(account.uuid)
                                           logMonthlyFeedListModel.selectCondition = name
                                           logMonthlyFeedListModel.clear()
                                           logMonthlyFeedListModel.getLatest()
                                       }
                    }
                    TimelineView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: swipeView.frameHeight
                        model: LogFeedListModel {
                            id: logMonthlyFeedListModel
                            targetDid: account.did
                            targetHandle: account.handle
                            targetAvatar: account.avatar
                            feedType: LogFeedListModel.MonthlyFeedType
                        }
                        accountDid: account.did
                        logMode: true
                        onRequestReply: (cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text) =>
                                        logViewDialog.requestReply(account.uuid, cid, uri, reply_root_cid, reply_root_uri, avatar, display_name, handle, indexed_at, text)
                        onRequestQuote: (cid, uri, avatar, display_name, handle, indexed_at, text) =>
                                        logViewDialog.requestQuote(account.uuid, cid, uri, avatar, display_name, handle, indexed_at, text)
                        onRequestViewImages: (index, paths, alts) => logViewDialog.requestViewImages(index, paths, alts)
                        onRequestAddMutedWord: (text) => logViewDialog.requestAddMutedWord(account.uuid, text)
                        onRequestUpdateThreadGate: (uri, threadgate_uri, type, rules, callback) =>
                                                   logViewDialog.requestUpdateThreadGate(account.uuid, uri, threadgate_uri, type, rules, callback)
                        onHoveredLinkChanged: logViewDialog.hoveredLink = hoveredLink
                    }
                }
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
