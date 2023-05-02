import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    width: 800
    height: 480
    visible: true
    title: qsTr("Hagromo")

    LoginDialog {
        id: login
        onAccepted: {

        }
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

                Label {
                    text: "did:"
                }
                Label {
                    text: login.session.did
                }
                Label {
                    text: "handle:"
                }
                Label {
                    text: login.session.handle
                }
                Label {
                    text: "email:"
                }
                Label {
                    text: login.session.email
                }
                Label {
                    text: "accessJwt:"
                }
                Label {
                    text: login.session.accessJwt
                }
                Label {
                    text: "refreshJwt:"
                }
                Label {
                    text: login.session.refreshJwt
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

            RowLayout {
                spacing: 3
                TimelineView {
                    Layout.preferredHeight: scrollView.height - scrollView.ScrollBar.horizontal.height
                    Layout.minimumWidth: 100
                    Layout.preferredWidth: 400
                    Layout.maximumWidth: 500
                }
                TimelineView {
                    Layout.preferredHeight: scrollView.height - scrollView.ScrollBar.horizontal.height
                    Layout.minimumWidth: 100
                    Layout.preferredWidth: 400
                    Layout.maximumWidth: 500
                }
            }
        }
    }
}
