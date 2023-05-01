import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.createsession 1.0

Dialog {
    id: root
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5

    property int parentWidth: parent.width

    property alias session: session

    CreateSession {
        id: session
        service: serviceTextInput.text
        identifier: idTextInput.text
        password: passwordTextInput.text

        onFinished: (success) => {
                      console.log("created:" + success)
                      if(success){
                          root.accept()
                      }else{
                          // NG
                      }
                  }
    }

    GridLayout {
        columns: 2
        columnSpacing: 10
        rowSpacing: 10

        Label {
            text: "Service"
        }
        TextInput {
            id: serviceTextInput
            Layout.minimumWidth: root.parentWidth * 0.5
            enabled: !session.running
            Label {
                anchors.fill: parent
                visible: parent.text.length === 0
                opacity: 0.5
                text: "https://bsky.social etc..."
            }
        }
        Label {
            text: "Identifier"
        }
        TextInput {
            id: idTextInput
            Layout.fillWidth: true
            enabled: !session.running
            Label {
                anchors.fill: parent
                visible: parent.text.length === 0
                opacity: 0.5
                text: "Handle or Email address or DID"
            }
        }
        Label {
            text: "Password"
        }
        TextInput {
            id: passwordTextInput
            Layout.fillWidth: true
            enabled: !session.running
            echoMode: TextInput.Password
        }

        Button {
            enabled: !session.running
            text: qsTr("Sign in")
            onClicked: session.create()
        }
    }

}
