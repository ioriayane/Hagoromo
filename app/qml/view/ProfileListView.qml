import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.recordoperator 1.0
import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"

ScrollView {
    id: profileListView
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    clip: true

    property string hoveredLink: ""
    property string userDid: ""     // 表示するアカウント
    property string accountDid: ""  // 認証しているアカウント
    property bool unfollowAndRemove: true   // アンフォローと同時にリストから消すか（検索結果は消さない）

    property alias listView: rootListView
    property alias model: rootListView.model
    property alias recordOperator: recordOperator

    signal requestViewProfile(string did)

    ListView {
        id: rootListView
        anchors.fill: parent
        anchors.rightMargin: parent.ScrollBar.vertical.width
        spacing: 5

        RecordOperator {
            id: recordOperator
            property string opeDid: ""
            onFinished: (success) => {
                            if(opeDid.length > 0){
                                model.getProfile(opeDid)
                            }else{
                                model.getLatest()
                            }
                        }
            function reflectAccount() {
                recordOperator.setAccount(rootListView.model.service, rootListView.model.did,
                                          rootListView.model.handle, rootListView.model.email,
                                          rootListView.model.accessJwt, rootListView.model.refreshJwt)
            }
        }

        header: ItemDelegate {
            width: rootListView.width
            height: AdjustedValues.h24
            display: AbstractButton.IconOnly
            icon.source: rootListView.model.running ? "" : "../images/expand_less.png"
            onClicked: rootListView.model.getLatest()

            BusyIndicator {
                anchors.centerIn: parent
                width: AdjustedValues.i24
                height: AdjustedValues.i24
                visible: rootListView.model.running
            }
        }
        footer: BusyIndicator {
            width: rootListView.width
            height: AdjustedValues.i24
            visible: rootListView.model.running && rootListView.model.rowCount() > 0
        }

        //        add: Transition {
        //            NumberAnimation { properties: "x"; from: rootListView.width; duration: 300 }
        //        }

        delegate: ClickableFrame {
            id: profileLayout
            clip: true
            style: "Post"
            topPadding: 10
            leftPadding: 10
            rightPadding: 10
            bottomPadding: 0

            property int layoutWidth: rootListView.width

            states: [
                State {
                    // 通信中
                    when: recordOperator.running || rootListView.model.running
                    PropertyChanges { target: editButton; iconText: "   " }
                    PropertyChanges { target: editButton; enabled: false }
                },
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
                    PropertyChanges { target: editButton; onClicked: {
                            if(unfollowAndRemove){
                                recordOperator.opeDid = ""
                            }else{
                                recordOperator.opeDid = model.did
                            }
                            recordOperator.reflectAccount()
                            recordOperator.deleteFollow(model.followingUri)
                            // 表示しているプロフィールが取得したアカウントと同じ場合はモデルからも消す
                            if(unfollowAndRemove){
                                rootListView.model.remove(model.did)
                            }
                        } }
                },
                State {
                    // フォローしていない
                    when: !model.following
                    PropertyChanges { target: editButton; iconText: qsTr("Follow") }
                    PropertyChanges { target: editButton; onClicked: {
                            recordOperator.opeDid = model.did
                            recordOperator.reflectAccount()
                            recordOperator.follow(model.did)
                        } }
                }
            ]

            RowLayout{
                AvatarImage {
                    id: postAvatarImage
                    Layout.preferredWidth: AdjustedValues.i36
                    Layout.preferredHeight: AdjustedValues.i36
                    Layout.alignment: Qt.AlignTop
                    source: model.avatar
                    onClicked: requestViewProfile(model.did)
                }

                ColumnLayout {
                    property int basisWidth: profileLayout.layoutWidth - profileLayout.leftPadding - profileLayout.rightPadding -
                                             postAvatarImage.width - parent.spacing
                    Label {
                        Layout.fillWidth: true
                        font.pointSize: AdjustedValues.f10
                        text: model.displayName

                        IconButton {
                            id: editButton
                            anchors.right: parent.right
                            height: AdjustedValues.b24
                            iconText: "   "
                            BusyIndicator {
                                anchors.fill: parent
                                visible: recordOperator.running
                            }
                        }
                    }
                    Label {
                        font.pointSize: AdjustedValues.f8
                        color: Material.color(Material.Grey)
                        text: "@" + model.handle
                    }
                    RowLayout {
                        visible: model.followedBy || model.muted
                        TagLabel {
                            visible: model.followedBy
                            source: ""
                            fontPointSize: AdjustedValues.f8
                            text: qsTr("Follows you")
                        }
                        TagLabel {
                            visible: model.muted
                            source: ""
                            fontPointSize: AdjustedValues.f8
                            text: qsTr("Muted user")
                        }
                    }
                    Label {
                        id: childRecordText
                        Layout.preferredWidth: parent.basisWidth
                        Layout.maximumWidth: parent.basisWidth
                        wrapMode: Text.WrapAnywhere
                        font.pointSize: AdjustedValues.f8
                        lineHeight: 1.3
                        textFormat: Text.StyledText
                        text: model.description

                        onHoveredLinkChanged: profileListView.hoveredLink = hoveredLink
                        onLinkActivated: (url) => Qt.openUrlExternally(url)
                    }
                }
            }
        }
    }
}
