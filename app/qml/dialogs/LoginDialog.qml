import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.createsession 1.0
import tech.relog.hagoromo.oauthlogin 1.0
import tech.relog.hagoromo.singleton 1.0

Dialog {
    id: loginDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5

    property int parentWidth: parent.width

    property alias session: session
    property alias oauthLogin: oauthLogin
    property alias serviceText: serviceTextInput.text
    property alias idText: idTextInput.text
    property alias passwordText: passwordTextInput.text
    // "oauth" or "password"
    property string authMethod: "oauth"
    readonly property bool useOAuth: authMethod === "oauth"
    readonly property bool running: session.running || oauthLogin.running

    signal errorOccurred(string code, string message)

    onClosed: {
        // ブラウザでの認可を待っていたら止める
        oauthLogin.cancel()
        mfaCodeTextInput.visible = false
        mfaCodeTextInput.text = ""
    }

    CreateSession {
        id: session
        service: serviceTextInput.text
        identifier: idTextInput.text
        password: passwordTextInput.text
        authFactorToken: mfaCodeTextInput.text

        onFinished: (success) => {
                      if(success){
                          loginDialog.accept()
                      }else{
                          // NG
                      }
                  }
        onErrorOccurred: (code, message) => {
                            if(code === "AuthFactorTokenRequired"){
                                mfaCodeTextInput.text = ""
                                mfaCodeTextInput.visible = true
                            }else{
                                loginDialog.errorOccurred(code, message)
                            }
                        }
    }

    OAuthLogin {
        id: oauthLogin
        service: serviceTextInput.text
        identifier: idTextInput.text

        onRequestOpenUrl: (url) => Qt.openUrlExternally(url)
        onFinished: (success) => {
                        if(success){
                            loginDialog.accept()
                        }
                    }
        onErrorOccurred: (code, message) => loginDialog.errorOccurred(code, message)
    }

    GridLayout {
        columns: 2
        columnSpacing: AdjustedValues.s10
        rowSpacing: AdjustedValues.s10

        Label {
            font.pointSize: AdjustedValues.f10
            text: qsTr("Login method")
        }
        RowLayout {
            enabled: !loginDialog.running
            ButtonGroup {
                id: authMethodButtonGroup
            }
            RadioButton {
                font.pointSize: AdjustedValues.f10
                text: qsTr("Browser (OAuth)")
                checked: loginDialog.useOAuth
                ButtonGroup.group: authMethodButtonGroup
                onClicked: loginDialog.authMethod = "oauth"
            }
            RadioButton {
                font.pointSize: AdjustedValues.f10
                text: qsTr("App password")
                checked: !loginDialog.useOAuth
                ButtonGroup.group: authMethodButtonGroup
                onClicked: loginDialog.authMethod = "password"
            }
        }

        Label {
            font.pointSize: AdjustedValues.f10
            text: qsTr("Service")
        }
        TextField {
            id: serviceTextInput
            Layout.minimumWidth: loginDialog.parentWidth
            enabled: !loginDialog.running
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
            enabled: !loginDialog.running
            placeholderText: loginDialog.useOAuth ? "Handle or DID" : "Handle or Email address or DID"
            font.pointSize: AdjustedValues.f10
        }
        Label {
            font.pointSize: AdjustedValues.f10
            text: qsTr("Password")
            visible: !loginDialog.useOAuth
        }
        TextField {
            id: passwordTextInput
            Layout.fillWidth: true
            enabled: !loginDialog.running
            visible: !loginDialog.useOAuth
            echoMode: TextInput.Password
            placeholderText: "The use of App Password is recommended."
            font.pointSize: AdjustedValues.f10
        }
        Label {
            font.pointSize: AdjustedValues.f10
            text: qsTr("2FA Confirmation")
            visible: mfaCodeTextInput.visible
        }
        TextField {
            id: mfaCodeTextInput
            Layout.fillWidth: true
            enabled: !loginDialog.running
            visible: false
            echoMode: TextInput.Password
            placeholderText: "Confirmation code"
            font.pointSize: AdjustedValues.f10
        }

        Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.maximumWidth: loginDialog.parentWidth * 1.3
            visible: loginDialog.useOAuth && !oauthLogin.running
            wrapMode: Text.WrapAnywhere
            font.pointSize: AdjustedValues.f8
            text: qsTr("Log in on the page opened in your web browser. When the session expires, you will need to log in again.")
        }
        ColumnLayout {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            visible: oauthLogin.running
            RowLayout {
                BusyIndicator {
                    Layout.preferredWidth: AdjustedValues.i24
                    Layout.preferredHeight: AdjustedValues.i24
                    running: oauthLogin.running
                }
                Label {
                    Layout.fillWidth: true
                    font.pointSize: AdjustedValues.f10
                    text: oauthLogin.authorizationUrl.length > 0 ?
                              qsTr("Waiting for authorization in your web browser...") :
                              qsTr("Preparing authorization...")
                }
            }
            Button {
                flat: true
                visible: oauthLogin.authorizationUrl.length > 0
                font.pointSize: AdjustedValues.f8
                text: qsTr("Open the browser again")
                onClicked: Qt.openUrlExternally(oauthLogin.authorizationUrl)
            }
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
            enabled: {
                if(loginDialog.running ||
                        serviceTextInput.text.length == 0 ||
                        idTextInput.text.length == 0){
                    return false
                }else if(loginDialog.useOAuth){
                    return true
                }else{
                    return !(passwordTextInput.text.length == 0 ||
                             (mfaCodeTextInput.visible && mfaCodeTextInput.text.length === 0))
                }
            }
            font.pointSize: AdjustedValues.f10
            text: qsTr("Login")
            onClicked: {
                if(loginDialog.useOAuth){
                    oauthLogin.start()
                }else{
                    session.create()
                }
            }
        }
    }
}
