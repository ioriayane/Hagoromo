import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.accountlistmodel 1.0

ApplicationWindow {
    width: 800
    height: 480
    visible: true
    title: qsTr("Hagromo")

    Material.theme: Material.Dark

    LoginDialog {
        id: login
        onAccepted: {
            accountListModel.updateAccount(session.service, session.identifier, session.password,
                                           session.did, session.handle, session.email,
                                           session.accessJwt, session.refreshJwt)
        }
    }

    AccountListModel {
        id: accountListModel
    }

    Component {
        id: timelineComponent
        TimelineView {
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0
        Rectangle {
            Layout.fillHeight: true
            Layout.minimumWidth: 64
            Layout.maximumWidth: 128
            color: "#00000000"
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 1
                Button {
                    Layout.fillWidth: true
                    display: AbstractButton.IconOnly
                    // display: AbstractButton.TextBesideIcon
                    icon.source: "images/edit.png"
                    text: qsTr("New Post")
                }

                Item {
                    Layout.preferredWidth: 1
                    Layout.fillHeight: true
                }

                Button {
                    Layout.fillWidth: true
                    display: AbstractButton.IconOnly
                    // display: AbstractButton.TextBesideIcon
                    icon.source: "images/add_user.png"
                    text: qsTr("Add user")

                    onClicked: {
                        repeater.append(0)
                    }
                }

                Button {
                    Layout.fillWidth: true
                    display: AbstractButton.IconOnly
                    // display: AbstractButton.TextBesideIcon
                    icon.source: "images/add_user.png"
                    text: qsTr("Add user")

                    onClicked: login.open()
                }

                Button {
                    Layout.fillWidth: true
                    display: AbstractButton.IconOnly
                    // display: AbstractButton.TextBesideIcon
                    icon.source: "images/settings.png"
                    text: qsTr("Settings")
                }
            }
        }
        Rectangle {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: "#999999"
        }
        ScrollView {
            id: scrollView
            Layout.fillWidth: true
            Layout.fillHeight: true
            ScrollBar.vertical.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.interactive: false
            ScrollBar.vertical.snapMode: ScrollBar.SnapAlways
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOn

            property int childHeight: scrollView.height - scrollView.ScrollBar.horizontal.height

            RowLayout {
                spacing: 3
                Repeater {
                    id: repeater
                    model: ListModel {}

                    function append(account_index){
                        // accountListModelで管理するアカウントのindexと表示に使うコンポを指定
                        repeater.model.append({
                                                  "account_index": account_index,
                                                  "component": timelineComponent
                                              })
                    }
                    onItemAdded: (index, item) => {
                                     //console.log("" + index + ":" + item + ":" + repeater.model.get(index).account_index)
                                     item.account_index = repeater.model.get(index).account_index
                                     item.sourceComponent = repeater.model.get(index).component
                                 }

                    Loader {
                        id: loader
                        Layout.preferredHeight: scrollView.childHeight
                        Layout.minimumWidth: 100
                        Layout.preferredWidth: 400
                        Layout.maximumWidth: 500

                        property int account_index: -1
                        onLoaded: {
                            //console.log("loader:" + loader.account_index)
                            if(loader.account_index < 0)
                                return
                            var i = loader.account_index
                            item.model.updateAccount(accountListModel.item(i, AccountListModel.ServiceRole),
                                                     accountListModel.item(i, AccountListModel.DidRole),
                                                     accountListModel.item(i, AccountListModel.HandleRole),
                                                     accountListModel.item(i, AccountListModel.EmailRole),
                                                     accountListModel.item(i, AccountListModel.AccessJwtRole),
                                                     accountListModel.item(i, AccountListModel.RefreshJwtRole))
                            item.model.getLatest()
                        }
                    }
                }

                //                TimelineView {
                //                    id: timeline
                //                    Layout.preferredHeight: scrollView.childHeight
                //                    Layout.minimumWidth: 100
                //                    Layout.preferredWidth: 400
                //                    Layout.maximumWidth: 500
                //                }
                //                TimelineView {
                //                    Layout.preferredHeight: scrollView.childHeight
                //                    Layout.minimumWidth: 100
                //                    Layout.preferredWidth: 400
                //                    Layout.maximumWidth: 500
                //                }

                // debug
                ListView {
                    Layout.preferredHeight: scrollView.childHeight
                    Layout.minimumWidth: 100
                    Layout.preferredWidth: 400
                    Layout.maximumWidth: 500
                    model: accountListModel
                    delegate: GridLayout {
                        columns: 2
                        Label {
                            text: "service:"
                        }
                        Label {
                            text: model.service
                        }
                        Label {
                            text: "identifier:"
                        }
                        Label {
                            text: model.identifier
                        }
                        Label {
                            text: "password:"
                        }
                        Label {
                            text: model.password
                        }
                        Label {
                            text: "did:"
                        }
                        Label {
                            text: model.did
                        }
                        Label {
                            text: "handle:"
                        }
                        Label {
                            text: model.handle
                        }
                        Label {
                            text: "email:"
                        }
                        Label {
                            text: model.email
                        }
                        Label {
                            text: "accessJwt:"
                        }
                        Label {
                            text: model.accessJwt
                        }
                        Label {
                            text: "refreshJwt:"
                        }
                        Label {
                            text: model.refreshJwt
                        }
                        Label {
                            text: "-"
                        }
                        Label {
                            text: "-"
                        }

                    }
                }
            }
        }
    }
}
