import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.accountlistmodel 1.0

ApplicationWindow {
    width: 800
    height: 480
    visible: true
    title: qsTr("Hagromo")

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

    RowLayout {
        anchors.fill: parent
        Rectangle {
            Layout.fillHeight: true
            Layout.minimumWidth: 64
            Layout.maximumWidth: 128
            border.width: 1
            ColumnLayout {
                anchors.fill: parent

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
                TimelineView {
                    Layout.preferredHeight: scrollView.childHeight
                    Layout.minimumWidth: 100
                    Layout.preferredWidth: 400
                    Layout.maximumWidth: 500
                }
                TimelineView {
                    Layout.preferredHeight: scrollView.childHeight
                    Layout.minimumWidth: 100
                    Layout.preferredWidth: 400
                    Layout.maximumWidth: 500
                }
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
