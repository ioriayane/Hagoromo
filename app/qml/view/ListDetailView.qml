import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.listitemlistmodel 1.0
import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"

ColumnLayout {
    id: listDetailView
    Layout.fillWidth: true
    Layout.fillHeight: true
    spacing: 0

    property string hoveredLink: ""

    property alias model: relayObject
    property string listUri: ""

    signal requestViewProfile(string did)

    signal errorOccured(string code, string message)
    signal back()

    QtObject {
        id: relayObject
        function rowCount() {
            return listItemListModel.rowCount();
        }
        function setAccount(service, did, handle, email, accessJwt, refreshJwt) {
            listItemListModel.setAccount(service, did, handle, email, accessJwt, refreshJwt)
        }
        function getLatest() {
            listItemListModel.getLatest()
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

            model: ListItemListModel {
                id: listItemListModel
                autoLoading: false
                uri: listDetailView.listUri
                onErrorOccured: (code, message) => listDetailView.errorOccured(code, message)
            }

            header: Item {
                width: rootListView.width
                height: AdjustedValues.h24

                BusyIndicator {
                    anchors.centerIn: parent
                    width: AdjustedValues.i24
                    height: AdjustedValues.i24
                    visible: listItemListModel.running
                }
            }

            delegate: ClickableFrame {
                id: listItemLayout
                clip: true
                style: "Post"
                topPadding: 10
                leftPadding: 10
                rightPadding: 10
                bottomPadding: 0

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
