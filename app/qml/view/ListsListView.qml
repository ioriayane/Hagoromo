import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.listslistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"

ScrollView {
    id: listsListView
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    clip: true

    property alias listView: rootListView
    property alias model: rootListView.model
    property int mode: 0    // 0:プロフィール用, 1:ミュートリスト一覧ダイアログ用, 2:ブロックリスト一覧ダイアログ用

    signal requestViewListDetail(string uri)

    signal scrollPositionChanged(bool top)

    ListView {
        id: rootListView
        anchors.fill: parent
        anchors.rightMargin: parent.ScrollBar.vertical.width
        spacing: 5
        maximumFlickVelocity: AdjustedValues.maximumFlickVelocity

        onAtYBeginningChanged: scrollPositionChanged(atYBeginning)
        onMovementStarted: scrollPositionChanged(atYBeginning)
        onMovementEnded: {
            if(atYEnd){
                listsListView.model.getNext()
            }
        }

        header: ItemDelegate {
            width: rootListView.width
            height: AdjustedValues.h24
            display: AbstractButton.IconOnly
            icon.source: rootListView.model.running ? "" : "../images/expand_less.png"
            onClicked: {
                rootListView.model.clear()
                rootListView.model.getLatest()
            }

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


        delegate: ClickableFrame {
            id: listsLayout
            contentWidth: contentRootLayout.implicitWidth
            contentHeight: contentRootLayout.implicitHeight
            clip: true
            style: "Post"
            topPadding: 10
            leftPadding: 10
            rightPadding: 10
            bottomPadding: 10

            property int layoutWidth: rootListView.width

            onClicked: (mouse) => listsListView.requestViewListDetail(model.uri)

            states: [
                State {
                    when: (listsListView.mode === 0)
                    PropertyChanges { target: statusLabelLayout; visible: true }
                    PropertyChanges { target: controlButton; visible: false }

                },
                State {
                    when: (listsListView.mode === 1)
                    PropertyChanges { target: statusLabelLayout; visible: false }
                    PropertyChanges {
                        target: controlButton
                        visible: true
                        iconText: model.muted ? qsTr("Unmute") : qsTr("Mute")
                        onClicked: {
                            console.log("mute " + model.index)
                            rootListView.model.mute(model.index)
                        }
                    }
                },
                State {
                    when: (listsListView.mode === 2)
                    PropertyChanges { target: statusLabelLayout; visible: false }
                    PropertyChanges {
                        target: controlButton
                        visible: true
                        iconText: model.blocked ? qsTr("Unblock") : qsTr("Block")
                        onClicked: {
                            console.log("block " + model.index)
                            rootListView.model.block(model.index)
                        }
                    }
                }
            ]

            RowLayout{
                id: contentRootLayout
                spacing: 10
                AvatarImage {
                    id: postAvatarImage
                    Layout.preferredWidth: AdjustedValues.i36
                    Layout.preferredHeight: AdjustedValues.i36
                    Layout.alignment: Qt.AlignTop
                    source: model.avatar
                    onClicked: (mouse) => listsLayout.clicked(mouse)
                }

                ColumnLayout {
                    property int basisWidth: listsLayout.layoutWidth - listsLayout.leftPadding - listsLayout.rightPadding -
                                             postAvatarImage.width - parent.spacing
                    Label {
                        Layout.preferredWidth: parent.basisWidth
                        font.pointSize: AdjustedValues.f10
                        text: model.name
                        IconButton {
                            id: controlButton
                            anchors.top: parent.top
                            anchors.right: parent.right
                            height: AdjustedValues.b24
                            enabled: !listsListView.model.running
                            iconText: "   "
                            font.pointSize: AdjustedValues.f8
                        }
                    }
                    Label {
                        Layout.preferredWidth: parent.basisWidth
                        color: Material.color(Material.Grey)
                        font.pointSize: AdjustedValues.f8
                        text: "by " + model.creatorDisplayName + " (" + model.creatorHandle + ")"
                    }
                    TagLabel {
                        fontPointSize: AdjustedValues.f8
                        color: Material.color(Material.Red,
                                              Material.theme === Material.Light ? Material.Shade100 : Material.Shade800)
                        text: "Moderation"
                        source: "../images/list.png"
                        visible: model.isModeration
                    }
                    RowLayout {
                        id: statusLabelLayout
                        visible: (listsListView.mode === 0)
                        TagLabel {
                            fontPointSize: AdjustedValues.f8
                            color: Material.color(Material.BlueGrey,
                                                  Material.theme === Material.Light ? Material.Shade100 : Material.Shade800)
                            text: qsTr("Muted")
                            source: "../images/list.png"
                            visible: model.muted
                        }
                        TagLabel {
                            fontPointSize: AdjustedValues.f8
                            color: Material.color(Material.BlueGrey,
                                                  Material.theme === Material.Light ? Material.Shade100 : Material.Shade800)
                            text: qsTr("Blocked")
                            source: "../images/list.png"
                            visible: model.blocked
                        }
                    }
                    Label {
                        Layout.preferredWidth: parent.basisWidth
                        font.pointSize: AdjustedValues.f8
                        visible: text.length > 0
                        text: model.description
                    }
                }
            }
        }
    }
}
