import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hagromo")

    LoginDialog {
        id: login
        onAccepted: {

        }
    }


    GridLayout {
        columns: 2

        Label {
            text: "start"
        }
        Button {
            text: "Login"
            onClicked: login.open()
        }

        Label {
            text: "did"
        }
        Label {
            text: login.session.did
        }
        Label {
            text: "handle"
        }
        Label {
            text: login.session.handle
        }
        Label {
            text: "email"
        }
        Label {
            text: login.session.email
        }
        Label {
            text: "accessJwt"
        }
        Label {
            text: login.session.accessJwt
        }
        Label {
            text: "refreshJwt"
        }
        Label {
            text: login.session.refreshJwt
        }
    }
}
