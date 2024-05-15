import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.accountlistmodel 1.0
import tech.relog.hagoromo.feedtypelistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../parts"

Dialog {
    id: addColumnDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Add column")

    property alias accountModel: accountList.model

    property alias selectedAccountIndex: accountList.currentIndex
    property int selectedType: 0
    property string selectedName: ""
    property string selectedUri: ""

    signal openSatisticsAndLogs(string account_uid)
    signal openDiscoverFeeds(string account_uuid)
    signal errorOccured(string account_uuid, string code, string message)

    onOpened: {
        if(accountList.currentIndex === -1){
            accountList.currentIndex = 0
        }
        changeColumnTypeView(accountList.currentIndex)
    }

    // アカウントの選択を変更したときにカラムタイプのリストをすり替える
    function changeColumnTypeView(index) {
        for(var i=0; i<repeater.count; i++){
            var item = repeater.itemAt(i)
            item.visible = (i === index)
            if(i === index){
                if(item.model.rowCount() === 2){
                    var service = accountModel.item(index, AccountListModel.ServiceRole)
                    var did = accountModel.item(index, AccountListModel.DidRole)
                    item.model.setAccount(service, did,
                                          accountModel.item(index, AccountListModel.HandleRole),
                                          accountModel.item(index, AccountListModel.EmailRole),
                                          accountModel.item(index, AccountListModel.AccessJwtRole),
                                          accountModel.item(index, AccountListModel.RefreshJwtRole))
                    item.model.getLatest()
                }

                // 切り替わったカラムリストの選択状態の更新と選択情報も切り替える
                item.listView.currentIndex = 0
                var c_index = item.listView.currentIndex
                addColumnDialog.selectedType = item.model.item(c_index, FeedTypeListModel.FeedTypeRole)
                addColumnDialog.selectedName = item.model.item(c_index, FeedTypeListModel.DisplayNameRole)
                addColumnDialog.selectedUri = item.model.item(c_index, FeedTypeListModel.UriRole)
            }
        }
    }

    ColumnLayout {
        RowLayout {
            spacing: 0

            ColumnLayout {
                Label {
                    font.pointSize: AdjustedValues.f10
                    text: qsTr("Account")
                }
                AccountList {
                    id: accountList
                    Layout.preferredWidth: 200 * AdjustedValues.ratio
                    Layout.preferredHeight: 300 * AdjustedValues.ratio
                    onClicked: (index) => changeColumnTypeView(index)
                }
            }
            Image {
                source: "../images/arrow_forward.png"
                Layout.preferredWidth: AdjustedValues.i24
                Layout.preferredHeight: AdjustedValues.i24
                layer.enabled: true
                layer.effect: ColorOverlay {
                    color: Material.color(Material.Grey)
                }
            }
            ColumnLayout {
                RowLayout {
                    Label {
                        Layout.fillWidth: true
                        font.pointSize: AdjustedValues.f10
                        text: qsTr("Column type")
                    }
                    IconButton {
                        Layout.preferredWidth: AdjustedValues.b30
                        Layout.preferredHeight: AdjustedValues.b24
                        iconSource: "../images/refresh.png"
                        iconSize: AdjustedValues.i16
                        onClicked: {
                            repeater.itemAt(accountList.currentIndex).model.clear()
                            changeColumnTypeView(accountList.currentIndex)
                        }
                    }
                }
                Item {
                    Layout.preferredWidth: 300 * AdjustedValues.ratio
                    Layout.preferredHeight: 300 * AdjustedValues.ratio
                    Repeater {
                        id: repeater
                        model: accountModel.count
                        onModelChanged: {
                            for(var i=0; i<repeater.count; i++){
                                repeater.itemAt(i).model.clear()
                            }
                        }

                        ScrollView {
                            id: typeScroll
                            anchors.fill: parent
                            //ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                            clip: true
                            visible: index === 0

                            property alias listView: typeList
                            property alias model: typeList.model

                            ListView {
                                id: typeList
                                maximumFlickVelocity: AdjustedValues.maximumFlickVelocity
                                model: FeedTypeListModel {
                                    onErrorOccured: (code, message) => {
                                                        var uuid = accountModel.item(accountList.currentIndex, AccountListModel.UuidRole)
                                                        addColumnDialog.errorOccured(uuid, code, message)
                                                    }
                                }
                                footer: BusyIndicator {
                                    id: busyIndicator
                                    width: typeList.width - typeScroll.ScrollBar.vertical.width
                                    height: AdjustedValues.i32
                                    visible: typeList.model ? typeList.model.running : false
                                }

                                delegate: ItemDelegate {
                                    width: typeList.width - typeScroll.ScrollBar.vertical.width
                                    height: implicitHeight * AdjustedValues.ratio
                                    highlighted: ListView.isCurrentItem
                                    onClicked: {
                                        if(model.feedType === 101){
                                            if(!typeList.model.running){
                                                var uuid = accountModel.item(accountList.currentIndex, AccountListModel.UuidRole)
                                                console.log("index=" + accountList.currentIndex + "," + uuid)
                                                addColumnDialog.openDiscoverFeeds(uuid)
                                            }
                                        }else{
                                            typeList.currentIndex = model.index

                                            addColumnDialog.selectedType = model.feedType
                                            addColumnDialog.selectedName = model.displayName
                                            addColumnDialog.selectedUri = model.uri
                                        }
                                    }
                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        spacing: 5
                                        AvatarImage {
                                            Layout.preferredWidth: AdjustedValues.i24
                                            Layout.preferredHeight: AdjustedValues.i24
                                            source: model.avatar ? model.avatar : ""
                                            altSource: {
                                                if(model.feedType === 0){
                                                    return "../images/home.png"
                                                }else if(model.feedType === 1){
                                                    return "../images/notification.png"
                                                }else if(model.feedType === 101){
                                                    return "../images/feed.png"
                                                }else{
                                                    return "../images/account_icon.png"
                                                }
                                            }
                                        }
                                        Label {
                                            font.pointSize: AdjustedValues.f10
                                            text: model.displayName ? model.displayName : ""
                                        }
                                        Label {
                                            color: Material.color(Material.Grey)
                                            font.pointSize: AdjustedValues.f10
                                            text: model.creatorDisplayName ? (model.creatorDisplayName.length > 0 ? "by @" + model.creatorDisplayName : model.creatorDisplayName) : ""
                                        }
                                        Item {
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 1
                                        }
                                    }
                                }

                                section.property: "group"
                                section.criteria: ViewSection.FullString
                                section.delegate: Rectangle {
                                    width: typeList.width - typeScroll.ScrollBar.vertical.width
                                    height: text.contentHeight * 1.3 * AdjustedValues.ratio
                                    color: Material.color(Material.BlueGrey)
                                    required property string section
                                    Text {
                                        id: text
                                        anchors.left: parent.left
                                        anchors.leftMargin: 5
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: parent.section
                                        font.bold: true
                                        font.pixelSize: AdjustedValues.f12
                                        color: "white"
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        RowLayout {
            spacing: 10
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Cancel")
                flat: true
                onClicked: addColumnDialog.reject()
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Logs")
                flat: true
                onClicked: {
                    var uuid = accountModel.item(accountList.currentIndex, AccountListModel.UuidRole)
                    addColumnDialog.openSatisticsAndLogs(uuid)
                }
            }
            Button {
                font.pointSize: AdjustedValues.f10
                enabled: accountList.currentIndex >= 0
                text: qsTr("Add")
                onClicked: addColumnDialog.accept()
            }
        }
    }
}
