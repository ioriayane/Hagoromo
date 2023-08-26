import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.createsession 1.0
import tech.relog.hagoromo.singleton 1.0

Dialog {
    id: loginDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5

    property int parentWidth: parent.width

    property alias session: session
    property alias serviceText: serviceTextInput.text
    property alias idText: idTextInput.text
    property alias passwordText: passwordTextInput.text

    CreateSession {
        id: session
        service: serviceTextInput.text
        identifier: idTextInput.text
        password: passwordTextInput.text

        onFinished: (success) => {
                      if(success){
                          loginDialog.accept()
                      }else{
                          // NG
                      }
                  }
        onErrorOccured: (message) => {console.log(message)}
    }

    GridLayout {
        columns: 2
        columnSpacing: AdjustedValues.s10
        rowSpacing: AdjustedValues.s10

        Label {
            font.pointSize: AdjustedValues.f10
            text: qsTr("Service")
        }
        TextField {
            id: serviceTextInput
            Layout.minimumWidth: loginDialog.parentWidth
            enabled: !session.running
            placeholderText: "https://bsky.social etc..."
            font.pointSize: AdjustedValues.f10
        }
        Label {
            font.pointSize: AdjustedValues.f10
            text: qsTr("Identifier")
        }
        TextField {
            id: idTextInput
            Layout.fillWidth: true
            enabled: !session.running
            placeholderText: "Handle or Email address or DID"
            font.pointSize: AdjustedValues.f10
        }
        Label {
            font.pointSize: AdjustedValues.f10
            text: qsTr("Password")
        }
        TextField {
            id: passwordTextInput
            Layout.fillWidth: true
            enabled: !session.running
            echoMode: TextInput.Password
            placeholderText: "The use of App Password is recommended."
            font.pointSize: AdjustedValues.f10
        }

        Button {
            Layout.alignment: Qt.AlignLeft
            flat: true
            font.pointSize: AdjustedValues.f10
            text: qsTr("Cancel")
            onClicked: loginDialog.close()
        }
        Button {
            Layout.alignment: Qt.AlignRight
            enabled: !(session.running || serviceTextInput.text.length == 0 || idTextInput.text.length == 0 || passwordTextInput.text.length == 0)
            font.pointSize: AdjustedValues.f10
            text: qsTr("Login")
            onClicked: session.create()
        }
    }
}
