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
                        console.log("CreateRecord::onFinished " + success)
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
                createRecord.setAccount(root.accountModel.item(row, AccountListModel.ServiceRole),
                                        root.accountModel.item(row, AccountListModel.DidRole),
                                        root.accountModel.item(row, AccountListModel.HandleRole),
                                        root.accountModel.item(row, AccountListModel.EmailRole),
                                        root.accountModel.item(row, AccountListModel.AccessJwtRole),
                                        root.accountModel.item(row, AccountListModel.RefreshJwtRole))

                createRecord.post(postText.text)
            }
        }
    }
}
