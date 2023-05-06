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
        TextField {
            id: serviceTextInput
            Layout.minimumWidth: root.parentWidth * 0.5
            enabled: !session.running
            placeholderText: "https://bsky.social etc..."
        }
        Label {
            text: "Identifier"
        }
        TextField {
            id: idTextInput
            Layout.fillWidth: true
            enabled: !session.running
            placeholderText: "Handle or Email address or DID"
        }
        Label {
            text: "Password"
        }
        TextField {
            id: passwordTextInput
            Layout.fillWidth: true
            enabled: !session.running
            echoMode: TextInput.Password
            placeholderText: "The use of App Password is recommended."
        }

        Button {
            enabled: !session.running
            text: qsTr("Login")
            onClicked: session.create()
        }
    }
}
