import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../parts"

ScrollView {
    id: generatorScrollView
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    clip: true

    property bool selectable: true
    property alias listView: generatorListView
    property alias model: generatorListView.model
    property alias currentIndex: generatorListView.currentIndex

    signal clicked(string display_name, string uri)
    signal requestRemoveGenerator(string uri)
    signal requestSaveGenerator(string uri)

    signal scrollPositionChanged(bool top)

    ListView {
        id: generatorListView
        clip: true
        onAtYBeginningChanged: scrollPositionChanged(atYBeginning)
        onMovementStarted: scrollPositionChanged(atYBeginning)
        onMovementEnded: {
            if(atYEnd){
                generatorListView.model.getNext()
            }
        }
        footer: BusyIndicator {
            width: generatorListView.width - generatorScrollView.ScrollBar.vertical.width
            height: AdjustedValues.h32
            visible: generatorListView.model.running
        }
        delegate: ItemDelegate {
            width: generatorListView.width - generatorScrollView.ScrollBar.vertical.width
            height: contentLayout.height + 10
            highlighted: ListView.isCurrentItem && generatorScrollView.selectable
            onClicked: {
                if(generatorScrollView.selectable){
                    generatorListView.currentIndex = model.index
                }
                generatorScrollView.clicked(model.displayName, model.uri)
            }
            states: [
                State {
                    when: generatorListView.model.running
                    PropertyChanges {
                        target: saveButton
                        iconSource: ""
                        enabled: false
                    }
                },
                State {
                    when: model.saving === true
                    PropertyChanges {
                        target: saveButton
                        iconSource: "../images/bookmark_add.png"
                        foreground: Material.color(Material.Pink)
                        onClicked: generatorScrollView.requestRemoveGenerator(model.uri)
                    }
                },
                State {
                    when: model.saving === false
                    PropertyChanges {
                        target: saveButton
                        iconSource: "../images/bookmark_add.png"
                        foreground: Material.color(Material.Grey)
                        onClicked: generatorScrollView.requestSaveGenerator(model.uri)
                    }
                }
            ]
            ColumnLayout {
                id: contentLayout
                GridLayout {
                    Layout.topMargin: 10
                    Layout.leftMargin: 10
                    Layout.rightMargin: 20
                    columns: 2
                    AvatarImage {
                        Layout.preferredWidth: AdjustedValues.i24
                        Layout.preferredHeight: AdjustedValues.i24
                        Layout.rowSpan: 2
                        source: model.avatar
                        altSource: "../images/account_icon.png"
                    }
                    Label {
                        Layout.fillWidth: true
                        font.pointSize: AdjustedValues.f10
                        text: model.displayName
                    }
                    Label {
                        color: Material.color(Material.Grey)
                        font.pointSize: AdjustedValues.f8
                        text: "by @" + model.creatorDisplayName
                    }
                }
                Label {
                    Layout.leftMargin: 10
                    Layout.rightMargin: 20
                    Layout.preferredWidth: generatorListView.width - 30
                    wrapMode: Text.WrapAnywhere
                    font.pointSize: AdjustedValues.f10
                    text: model.description
                }
                RowLayout {
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    Image {
                        Layout.preferredWidth: AdjustedValues.i16
                        Layout.preferredHeight: AdjustedValues.i16
                        source: "../images/like.png"
                        layer.enabled: true
                        layer.effect: ColorOverlay {
                            color: Material.color(Material.Pink)
                        }
                    }
                    Label {
                        Layout.fillWidth: true
                        font.pointSize: AdjustedValues.f8
                        text: model.likeCount
                    }
                }
            }
            IconButton {
                id: saveButton
                anchors.top: parent.top
                anchors.topMargin: 10
                anchors.right: parent.right
                anchors.rightMargin: 20
                width: AdjustedValues.b36
                height: AdjustedValues.b26
                display: AbstractButton.IconOnly
                iconSize: AdjustedValues.i18
                BusyIndicator {
                    anchors.fill: parent
                    visible: generatorListView.model.running
                }
            }
        }
    }
}
