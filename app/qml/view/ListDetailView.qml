import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.recordoperator 1.0
import tech.relog.hagoromo.listitemlistmodel 1.0
import tech.relog.hagoromo.systemtool 1.0
import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"

ColumnLayout {
    id: listDetailView
    Layout.fillWidth: true
    Layout.fillHeight: true
    spacing: 0

    property string hoveredLink: ""

    property string listUri: ""
    property alias model: relayObject

    signal requestViewProfile(string did)
    signal requestViewListFeed(string uri, string name)
    signal requestEditList(string uri, string avatar, string name, string description)

    signal errorOccured(string code, string message)
    signal back()


    RecordOperator {
        id: recordOperator
        property string accountHandle: ""
        onFinished: (success) => {
                        if(success){
                            listDetailView.back()
                        }
                    }
        onErrorOccured: (code, message) => listDetailView.errorOccured(code, message)
    }

    SystemTool {
        id: systemTool
    }

    QtObject {
        id: relayObject
        function rowCount() {
            return listItemListModel.rowCount();
        }
        function setAccount(service, did, handle, email, accessJwt, refreshJwt) {
            listItemListModel.setAccount(service, did, handle, email, accessJwt, refreshJwt)
            recordOperator.setAccount(service, did, handle, email, accessJwt, refreshJwt)
            recordOperator.accountHandle = handle
        }
        function getLatest() {
            listItemListModel.getLatest()
        }
        function getNext() {
            listItemListModel.getNext()
        }
    }

    Frame {
        Layout.fillWidth: true
        leftPadding: 0
        topPadding: 0
        rightPadding: 10
        bottomPadding: 0

        RowLayout {
            IconButton {
                Layout.preferredWidth: AdjustedValues.b30
                Layout.preferredHeight: AdjustedValues.b30
                flat: true
                iconSource: "../images/arrow_left_single.png"
                onClicked: listDetailView.back()
            }
            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 10
                font.pointSize: AdjustedValues.f10
                text: qsTr("List")
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.topMargin: 5
        Layout.leftMargin: 5
        Layout.rightMargin: 5
        Layout.bottomMargin: 5
        AvatarImage {
            id: avatarImage
            Layout.preferredWidth: AdjustedValues.i48
            Layout.preferredHeight: AdjustedValues.i48
            Layout.rowSpan: 2
            source: listItemListModel.avatar
        }
        ColumnLayout {
            property int basisWidth: listDetailView.width - avatarImage.width -
                                     parent.spacing - parent.Layout.leftMargin - parent.Layout.rightMargin
            Label {
                Layout.preferredWidth: parent.basisWidth
                font.pointSize: AdjustedValues.f12
                elide: Text.ElideRight
                text: listItemListModel.name

                IconButton {
                    id: editButton
                    anchors.top: parent.top
                    anchors.right: parent.right
                    height: AdjustedValues.b24
                    iconText: qsTr("Edit")
                    visible: listItemListModel.mine
                    onClicked: listDetailView.requestEditList(listDetailView.listUri, avatarImage.source,
                                                              listItemListModel.name, listItemListModel.description)
                    BusyIndicator {
                        anchors.fill: parent
                        visible: recordOperator.running
                    }
                }
            }
            Label {
                Layout.preferredWidth: parent.basisWidth
                elide: Text.ElideRight
                font.pointSize: AdjustedValues.f8
                color: Material.color(Material.Grey)
                text: listItemListModel.creatorHandle.length == 0 ?
                          "" : "by " + listItemListModel.creatorDisplayName + " (" + listItemListModel.creatorHandle + ")"
            }
            TagLabel {
                fontPointSize: AdjustedValues.f8
                color: Material.color(Material.Red,
                                      Material.theme === Material.Light ? Material.Shade100 : Material.Shade800)
                text: "Moderation"
                source: "../images/list.png"
                visible: listItemListModel.isModeration
            }
            RowLayout {
                TagLabel {
                    fontPointSize: AdjustedValues.f8
                    color: Material.color(Material.BlueGrey,
                                          Material.theme === Material.Light ? Material.Shade100 : Material.Shade800)
                    text: qsTr("Muted")
                    source: "../images/list.png"
                    visible: listItemListModel.muted
                }
                TagLabel {
                    fontPointSize: AdjustedValues.f8
                    color: Material.color(Material.BlueGrey,
                                          Material.theme === Material.Light ? Material.Shade100 : Material.Shade800)
                    text: qsTr("Blocked")
                    source: "../images/list.png"
                    visible: listItemListModel.blocked
                }
            }
            Label {
                Layout.preferredWidth: parent.basisWidth
                font.pointSize: AdjustedValues.f8
                //visible: text.length > 0
                text: listItemListModel.description

                IconButton {
                    id: moreButton
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: AdjustedValues.b24
                    iconSource: "../images/more.png"
                    iconSize: AdjustedValues.i16
                    foreground: Material.color(Material.Grey)
                    onClicked: morePopup.open()
                    Menu {
                        id: morePopup
                        width: copyOfficialUrlMenuItem.implicitWidth
                        MenuItem {
                            id: copyOfficialUrlMenuItem
                            text: qsTr("Copy Official Url")
                            icon.source: "../images/copy.png"
                            onTriggered: systemTool.copyToClipboard(listItemListModel.getOfficialUrl())
                        }
                        MenuItem {
                            text: qsTr("Open in new col")
                            icon.source: "../images/add.png"
                            onTriggered: listDetailView.requestViewListFeed(listDetailView.listUri, listItemListModel.name)
                        }
                        MenuItem {
                            text: qsTr("Open in Official")
                            icon.source: "../images/open_in_other.png"
                            onTriggered: Qt.openUrlExternally(listItemListModel.getOfficialUrl())
                        }
                        MenuSeparator {}
                        MenuItem {
                            enabled: listItemListModel.mine
                            text: qsTr("Delete list")
                            icon.source: "../images/delete.png"
                            onTriggered: recordOperator.deleteList(listDetailView.listUri)
                        }
                        MenuSeparator {}
                        MenuItem {
                            enabled: listItemListModel.isModeration
                            text: listItemListModel.muted ? qsTr("Unmute list") : qsTr("Mute list")
                            icon.source: "../images/account_off.png"
                            onTriggered: listItemListModel.mute()
                        }
                        MenuItem {
                            enabled: listItemListModel.isModeration
                            text: listItemListModel.blocked ? qsTr("Unblock list") : qsTr("Block list")
                            icon.source: "../images/block.png"
                            onTriggered: listItemListModel.block()
                        }
                    }
                }
            }
        }
    }

    ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true

        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        clip: true

        ListView {
            id: rootListView
            anchors.fill: parent
            anchors.rightMargin: parent.ScrollBar.vertical.width
            maximumFlickVelocity: AdjustedValues.maximumFlickVelocity

            model: ListItemListModel {
                id: listItemListModel
                autoLoading: false
                uri: listDetailView.listUri
                property bool mine: (creatorHandle === recordOperator.accountHandle) &&  recordOperator.accountHandle.length > 0
                onErrorOccured: (code, message) => listDetailView.errorOccured(code, message)
            }

            onMovementEnded: {
                if(atYEnd){
                    listItemListModel.getNext()
                }
            }

            header: Item {
                width: rootListView.width
                height: AdjustedValues.h24

                Label {
                    anchors.left: parent.left
                    anchors.leftMargin: 10 * AdjustedValues.ratio
                    anchors.verticalCenter: parent.verticalCenter
                    font.pointSize: AdjustedValues.f8
                    text: qsTr("Users")
                }
                BusyIndicator {
                    anchors.fill: parent
                    visible: listItemListModel.running || recordOperator.running
                }
            }

            delegate: ClickableFrame {
                id: listItemLayout
                clip: true
                style: "Post"
                topPadding: 10
                leftPadding: 10
                rightPadding: 10
                bottomPadding: 10

                property int layoutWidth: rootListView.width
                property var userLabels: [] //model.labels


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
                        property int basisWidth: listItemLayout.layoutWidth - listItemLayout.leftPadding - listItemLayout.rightPadding -
                                                 postAvatarImage.width - parent.spacing
                        Label {
                            Layout.fillWidth: true
                            font.pointSize: AdjustedValues.f10
                            text: model.displayName
                        }
                        Label {
                            font.pointSize: AdjustedValues.f8
                            color: Material.color(Material.Grey)
                            text: "@" + model.handle
                        }
                        //                        RowLayout {
                        //                            visible: model.followedBy || model.muted
                        //                            TagLabel {
                        //                                visible: model.followedBy
                        //                                source: ""
                        //                                fontPointSize: AdjustedValues.f8
                        //                                text: qsTr("Follows you")
                        //                            }
                        //                            TagLabel {
                        //                                visible: model.muted
                        //                                source: ""
                        //                                fontPointSize: AdjustedValues.f8
                        //                                text: qsTr("Muted user")
                        //                            }
                        //                        }
                        TagLabelLayout {
                            Layout.preferredWidth: parent.basisWidth
                            visible: count > 0
                            model: listItemLayout.userLabels
                        }
                        Label {
                            Layout.preferredWidth: parent.basisWidth
                            Layout.maximumWidth: parent.basisWidth
                            wrapMode: Text.WrapAnywhere
                            font.pointSize: AdjustedValues.f8
                            lineHeight: 1.3
                            textFormat: Text.StyledText
                            text: model.description

                            onHoveredLinkChanged: listDetailView.hoveredLink = hoveredLink
                            onLinkActivated: (url) => Qt.openUrlExternally(url)
                        }
                    }
                }
            }
        }
    }
}
