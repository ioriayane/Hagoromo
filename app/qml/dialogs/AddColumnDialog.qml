import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.accountlistmodel 1.0
import tech.relog.hagoromo.feedtypelistmodel 1.0

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

    signal openDiscoverFeeds(int account_index)

    onOpened: {
        if(accountList.currentIndex === -1){
            accountList.currentIndex = 0
        }
        for(var i=0; i<repeater.count; i++){
            repeater.itemAt(i).model.clear()
        }
        changeColumnTypeView(accountList.currentIndex)
    }

    // アカウントの選択を変更したときにカラムタイプのリストをすり替える
    function changeColumnTypeView(index) {
        for(var i=0; i<repeater.count; i++){
            var item = repeater.itemAt(i)
            item.visible = (i === index)
            if(i === index){
                if(item.model.count === 2){
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
                    text: qsTr("Account")
                }
                ScrollView {
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: 300
                    //ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    clip: true

                    ListView {
                        id: accountList
                        delegate: ItemDelegate {
                            width: accountList.width
                            highlighted: ListView.isCurrentItem
                            onClicked: {
                                accountList.currentIndex = model.index
                                changeColumnTypeView(model.index)
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 5
                                AvatarImage {
                                    Layout.preferredWidth: 24
                                    Layout.preferredHeight: 24
                                    source: model.avatar
                                }
                                Label {
                                    text: model.handle
                                    elide: Text.ElideRight
                                }
                                Item {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 1
                                }
                            }
                        }
                    }
                }
            }
            Image {
                source: "../images/arrow_forward.png"
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                layer.enabled: true
                layer.effect: ColorOverlay {
                    color: Material.color(Material.Grey)
                }
            }
            ColumnLayout {
                Label {
                    text: qsTr("Column type")
                }
                Item {
                    Layout.preferredWidth: 300
                    Layout.preferredHeight: 300
                    Repeater {
                        id: repeater
                        model: accountModel.count
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
                                model: FeedTypeListModel { }
                                footer: ItemDelegate {
                                    width: typeList.width
                                    BusyIndicator {
                                        id: busyIndicator
                                        anchors.centerIn: parent
                                        height: 32
                                        visible: typeList.model ? typeList.model.running : false
                                    }
                                    Label {
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.left: parent.left
                                        anchors.leftMargin: 15
                                        visible: !busyIndicator.visible
                                        text: qsTr("Discover Feeds")
                                    }
                                    onClicked: {
                                        if(busyIndicator.visible){
                                            return
                                        }
                                        addColumnDialog.openDiscoverFeeds(accountList.currentIndex)
                                    }
                                }

                                delegate: ItemDelegate {
                                    width: typeList.width
                                    highlighted: ListView.isCurrentItem
                                    onClicked: {
                                        typeList.currentIndex = model.index

                                        addColumnDialog.selectedType = model.feedType
                                        addColumnDialog.selectedName = model.displayName
                                        addColumnDialog.selectedUri = model.uri
                                    }
                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        spacing: 5
                                        AvatarImage {
                                            Layout.preferredWidth: 24
                                            Layout.preferredHeight: 24
                                            source: model.avatar
                                            altSource: {
                                                if(model.feedType === 0){
                                                    return "../images/home.png"
                                                }else if(model.feedType === 1){
                                                    return "../images/notification.png"
                                                }else{
                                                    return "../images/account_icon.png"
                                                }
                                            }
                                        }
                                        Label {
                                            font.pointSize: 10
                                            text: model.displayName
                                        }
                                        Label {
                                            color: Material.color(Material.Grey)
                                            font.pointSize: 10
                                            text: model.creatorDisplayName.length > 0 ? "by @" + model.creatorDisplayName : model.creatorDisplayName
                                        }
                                        Item {
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 1
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        RowLayout {
            Button {
                text: qsTr("Cancel")
                flat: true
                onClicked: addColumnDialog.reject()
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                enabled: accountList.currentIndex >= 0
                text: qsTr("Add")
                onClicked: addColumnDialog.accept()
            }
        }
    }
}
