import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.createrecord 1.0
import tech.relog.hagoromo.accountlistmodel 1.0

Dialog {
    id: root
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5

    property int parentWidth: parent.width
    property alias accountModel: accountCombo.model

    CreateRecord {
        id: createRecord
        onFinished: (success) => {
                        if(success){
                            postText.clear()
                            root.close()
                        }
                    }
    }

    ColumnLayout {
        ComboBox {
            id: accountCombo
            Layout.preferredWidth: 150
            textRole: "handle"
            valueRole: "did"
            delegate: ItemDelegate {
                text: model.handle
                width: parent.width
                onClicked: accountCombo.currentIndex = model.index
            }
        }
        TextField {
            id: postText
            Layout.preferredWidth: root.parentWidth * 0.5
            Layout.preferredHeight: 100
            verticalAlignment: TextInput.AlignTop
            wrapMode: TextInput.WordWrap
        }
        Label {
            Layout.alignment: Qt.AlignRight
            font.pointSize: 8
            text: 300 - postText.text.length
        }

        Button {
            Layout.alignment: Qt.AlignRight
            enabled: postText.text.length > 0
            text: qsTr("Post")
            onClicked: {
                var row = accountCombo.currentIndex;
                createRecord.service = root.accountModel.item(row, AccountListModel.ServiceRole)
                createRecord.did = root.accountModel.item(row, AccountListModel.DidRole)
                createRecord.handle = root.accountModel.item(row, AccountListModel.HandleRole)
                createRecord.email = root.accountModel.item(row, AccountListModel.EmailRole)
                createRecord.accessJwt = root.accountModel.item(row, AccountListModel.AccessJwtRole)
                createRecord.refreshJwt = root.accountModel.item(row, AccountListModel.RefreshJwtRole)

                createRecord.post(postText.text)
            }
        }
    }
}
