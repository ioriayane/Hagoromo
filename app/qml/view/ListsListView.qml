import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

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
            clip: true
            style: "Post"
            topPadding: 10
            leftPadding: 10
            rightPadding: 10
            bottomPadding: 10

            property int layoutWidth: rootListView.width

            onClicked: (mouse) => listsListView.requestViewListDetail(model.uri)

            RowLayout{
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
