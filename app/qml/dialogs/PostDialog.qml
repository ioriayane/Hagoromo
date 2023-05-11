import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.createrecord 1.0
import tech.relog.hagoromo.accountlistmodel 1.0

import "../parts"

Dialog {
    id: postDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5

    property int parentWidth: parent.width
    property alias accountModel: accountCombo.model

    property string defaultAccountUuid: ""
    property string postType: "normal"  // normal, reply, quote

    property string replyCid: ""
    property string replyUri: ""
    property string replyAvatar: ""
    property string replyDisplayName: ""
    property string replyHandle: ""
    property string replyIndexedAt: ""
    property string replyText: ""

    onOpened: {
        var i = accountModel.indexAt(defaultAccountUuid)
        if(i >= 0){
            accountCombo.currentIndex = i
        }
    }
    onClosed: {
        defaultAccountUuid = ""
        postType = "normal"
        replyCid = ""
        replyUri = ""
        replyAvatar = ""
        replyDisplayName = ""
        replyHandle = ""
        replyIndexedAt = ""
        replyText = ""
    }

    CreateRecord {
        id: createRecord
        onFinished: (success) => {
                        console.log("CreateRecord::onFinished " + success)
                        if(success){
                            postText.clear()
                            postDialog.close()
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
            Layout.preferredWidth: postDialog.parentWidth * 0.5
            Layout.preferredHeight: 100
            verticalAlignment: TextInput.AlignTop
            wrapMode: TextInput.WordWrap
        }
        Label {
            Layout.alignment: Qt.AlignRight
            font.pointSize: 8
            text: 300 - postText.text.length
        }

        Frame {
            id: quoteFrame
            Layout.preferredWidth: postText.width
            visible: postType === "quote"
            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    Image {
                        id: avatorImage
                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 16
                        source: replyAvatar
                    }
                    Author {
                        displayName: replyDisplayName
                        handle: replyHandle
                        indexedAt: replyIndexedAt
                    }
                }
                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WrapAnywhere
                    font.pointSize: 10
                    text: replyText
                }
            }
        }

        Button {
            Layout.alignment: Qt.AlignRight
            enabled: postText.text.length > 0
            text: qsTr("Post")
            onClicked: {
                var row = accountCombo.currentIndex;
                createRecord.setAccount(postDialog.accountModel.item(row, AccountListModel.ServiceRole),
                                        postDialog.accountModel.item(row, AccountListModel.DidRole),
                                        postDialog.accountModel.item(row, AccountListModel.HandleRole),
                                        postDialog.accountModel.item(row, AccountListModel.EmailRole),
                                        postDialog.accountModel.item(row, AccountListModel.AccessJwtRole),
                                        postDialog.accountModel.item(row, AccountListModel.RefreshJwtRole))
                if(postType === "quote"){
                    createRecord.setQuote(replyCid, replyUri)
                }
                createRecord.post(postText.text)
            }
        }
    }
}
