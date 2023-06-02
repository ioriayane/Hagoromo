import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import "../parts"

Dialog {
    id: addColumnDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Add column")

    property alias accountModel: accountList.model

    property alias selectedAccountIndex: accountList.currentIndex
    property alias selectedTypeIndex: typeList.currentIndex

    ColumnLayout {
        RowLayout {
            spacing: 0

            ColumnLayout {
                Label {
                    text: qsTr("Account")
                }
                ListView {
                    id: accountList
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: 300
                    delegate: ItemDelegate {
                        width: accountList.width
                        highlighted: ListView.isCurrentItem
                        onClicked: accountList.currentIndex = model.index

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 5
                            AvatarImage {
                                Layout.preferredWidth: 24
                                Layout.preferredHeight: 24
                                source: model.avatar
                            }
                            Label {
                                text: model.handle
                                elide: Text.ElideRight
                            }
                            Item {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 1
                            }
                        }
                    }
                }
            }
            Image {
                source: "../images/arrow_forward.png"
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                layer.enabled: true
                layer.effect: ColorOverlay {
                    color: Material.color(Material.Grey)
                }
            }
            ColumnLayout {
                Label {
                    text: qsTr("Column type")
                }
                ListView {
                    id: typeList
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: 300
                    model: [qsTr("Following"), qsTr("Notification")]
                    delegate: ItemDelegate {
                        width: typeList.width
                        text: modelData
                        highlighted: ListView.isCurrentItem
                        onClicked: typeList.currentIndex = model.index
                    }
                }
            }
        }
        RowLayout {
            Button {
                text: qsTr("Cancel")
                flat: true
                onClicked: addColumnDialog.reject()
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                enabled: accountList.currentIndex >= 0 && typeList.currentIndex >= 0
                text: qsTr("Add")
                onClicked: addColumnDialog.accept()
            }
        }
    }
}
