import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import "../parts"
import "../controls"

ScrollView {
    id: profileListView
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    clip: true

    property string accountDid: ""  // 認証しているアカウント

    property alias listView: rootListView
    property alias model: rootListView.model

    ListView {
        id: rootListView
        anchors.fill: parent
        anchors.rightMargin: parent.ScrollBar.vertical.width
        spacing: 5

        header: ItemDelegate {
            width: rootListView.width
            height: 24
            display: AbstractButton.IconOnly
            icon.source: rootListView.model.running ? "" : "../images/expand_less.png"
            onClicked: rootListView.model.getLatest()

            BusyIndicator {
                anchors.centerIn: parent
                width: 24
                height: 24
                visible: rootListView.model.running
            }
        }
        footer: BusyIndicator {
            width: rootListView.width
            height: 24
            visible: rootListView.model.running && rootListView.model.rowCount() > 0
        }

        delegate: ClickableFrame {
            id: profileLayout
            width: rootListView.width
            clip: true
            style: "Post"
            topPadding: 10
            leftPadding: 10
            rightPadding: 10
            bottomPadding: 0

            states: [
                State {
                    // 自分
                    when: model.did === profileListView.accountDid
                    PropertyChanges { target: editButton; visible: false }
                },
                State {
                    // 取得したアカウントでフォローしている
                    when: model.following
                    PropertyChanges { target: editButton; iconText: qsTr("Following") }
                    PropertyChanges { target: editButton; highlighted: true }
                },
                State {
                    // フォローしていない
                    when: !model.following
                    PropertyChanges { target: editButton; iconText: qsTr("Follow") }
                }
            ]

            RowLayout{
                AvatarImage {
                    id: postAvatarImage
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 36
                    Layout.alignment: Qt.AlignTop
                    source: model.avatar
                }

                ColumnLayout {
                    property int basisWidth: profileLayout.width - profileLayout.leftPadding - profileLayout.rightPadding -
                                             postAvatarImage.width - parent.spacing
                    Label {
                        Layout.fillWidth: true
                        font.pointSize: 10
                        text: model.displayName

                        RowLayout {
                            anchors.right: parent.right
                            Label {
                                Layout.alignment: Qt.AlignVCenter
                                visible: model.followedBy
                                font.pointSize: 8
                                color: Material.accentColor
                                text: qsTr("Follows you")
                            }
                            IconButton {
                                id: editButton
                                Layout.preferredHeight: 24
                                iconText: "   "
                                BusyIndicator {
                                    anchors.fill: parent
                                    visible: false //recordOperator.running || userProfile.running
                                }
                            }
                        }
                    }
                    Label {
                        font.pointSize: 8
                        color: Material.color(Material.Grey)
                        text: "@" + model.handle
                    }
                    Label {
                        id: childRecordText
                        Layout.preferredWidth: parent.basisWidth
                        Layout.maximumWidth: parent.basisWidth
                        wrapMode: Text.WrapAnywhere
                        font.pointSize: 8
                        lineHeight: 1.3
                        text: model.description
                    }
                }
            }
        }
    }
}
