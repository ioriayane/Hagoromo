import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15

import tech.relog.hagoromo.singleton 1.0

Item {
    id: aggregatedAavatarImages
    property var avatars: []
    property var displayNames: []
    property var dids: []
    property var handles: []
    property var indexedAts: []
    property bool lineMode: true
    clip: true
    height: aggregatedAavatarImages.lineMode ? lineLayout.height : listLayout.height

    signal selectAvatar(string did)

    Behavior on height {
        NumberAnimation { duration: 300 }
    }
    onAvatarsChanged: updateList()
    onDidsChanged: updateList()
    onHandlesChanged: updateList()

    function updateList() {
        if(avatars.length === dids.length &&
                dids.length === handles.length &&
                handles.length === indexedAts.length){
            userListModel.clear()
            for(var i=0; i<avatars.length; i++){
                userListModel.append({
                                         avatar: avatars[i],
                                         displayName: displayNames[i],
                                         did: dids[i],
                                         handle: handles[i],
                                         indexedAt: indexedAts[i]
                                     })
            }
        }
    }

    RowLayout {
        id: lineLayout
        anchors.left: parent.left
        anchors.right: parent.right
        visible: aggregatedAavatarImages.lineMode
        spacing: AdjustedValues.s5
        Repeater {
            id: lineRepeater
            model: aggregatedAavatarImages.avatars
            property int thresh: 10
            AvatarImage {
                Layout.preferredWidth: AdjustedValues.i16
                Layout.preferredHeight: AdjustedValues.i16
                source: modelData
                visible: (model.index < lineRepeater.thresh)
                onClicked: {
                    aggregatedAavatarImages.selectAvatar(aggregatedAavatarImages.dids[model.index])
                }
            }
        }
        Label {
            font.pointSize: AdjustedValues.f8
            visible: (lineRepeater.count > lineRepeater.thresh)
            text: "+" + (lineRepeater.count - lineRepeater.thresh)
        }
        MouseArea {
            Layout.fillWidth: true
            Layout.preferredHeight: AdjustedValues.i16
            onClicked: aggregatedAavatarImages.lineMode = !aggregatedAavatarImages.lineMode
            Image {
                width: AdjustedValues.i16
                height: AdjustedValues.i16
                fillMode: Image.PreserveAspectFit
                source: "../images/expand_more.png"
                layer.enabled: true
                layer.effect: ColorOverlay {
                    color: Material.foreground
                }
            }
        }
    }
    ColumnLayout {
        id: listLayout
        anchors.left: parent.left
        anchors.right: parent.right
        visible: !aggregatedAavatarImages.lineMode
        RowLayout {
            Label {
                font.pointSize: AdjustedValues.f8
                text: qsTr("Hide")
            }
            MouseArea {
                Layout.fillWidth: true
                Layout.preferredHeight: AdjustedValues.i16
                onClicked: aggregatedAavatarImages.lineMode = !aggregatedAavatarImages.lineMode
                Image {
                    width: AdjustedValues.i16
                    height: AdjustedValues.i16
                    fillMode: Image.PreserveAspectFit
                    source: "../images/expand_less.png"
                    layer.enabled: true
                    layer.effect: ColorOverlay {
                        color: Material.foreground
                    }
                }
            }
        }

        Repeater {
            id: userListRepeater
            model: ListModel {
                id: userListModel
            }
            RowLayout {
                AvatarImage {
                    id: postAvatarImage
                    Layout.preferredWidth: AdjustedValues.i16
                    Layout.preferredHeight: AdjustedValues.i16
                    source: model.avatar
                    onClicked: aggregatedAavatarImages.selectAvatar(model.did)
                }
                Author {
                    id: postAuthor
                    layoutWidth: aggregatedAavatarImages.width - postAvatarImage.width - parent.spacing
                    displayName: model.displayName
                    handle: model.handle
                    indexedAt: model.indexedAt
                }
            }
        }
    }
}
