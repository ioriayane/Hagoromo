import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "../controls"

Rectangle {
    id: imageFullView
    color: "#aa000000"

    property var sources: []
    property var alts: []

    function open(index, sources, alts){
        imageFullView.sources = sources
        imageFullView.alts = alts
        imageFullView.visible = true
        imageFullListView.currentIndex = index
    }

    Shortcut {  // Close
        enabled: imageFullView.visible
        sequence: "Esc"
        onActivated: imageFullView.visible = false
    }
    Shortcut {
        enabled: imageFullView.visible && leftMoveButton.enabled
        sequence: "left"
        onActivated: leftMoveButton.clicked()
    }
    Shortcut {
        enabled: imageFullView.visible && righttMoveButton.enabled
        sequence: "right"
        onActivated: righttMoveButton.clicked()
    }

    MouseArea {
        anchors.fill: parent
        onClicked: (mouse) => imageFullView.visible = false
    }
    ListView {
        id: imageFullListView
        anchors.fill: parent
        snapMode: ListView.SnapOneItem
        orientation: ListView.Horizontal
        highlightMoveDuration: 500
        interactive: false
        maximumFlickVelocity: AdjustedValues.maximumFlickVelocity

        model: imageFullView.sources
        delegate: ColumnLayout {
            width: imageFullListView.width
            height: imageFullListView.height
            spacing: 0
            ImageWithIndicator {
                id: image
                Layout.preferredWidth: imageFullListView.width
//                Layout.preferredHeight: imageFullListView.height - altMessage.height - 10
                Layout.fillHeight: true
                fillMode: Image.PreserveAspectFit
                source: modelData
                MouseArea {
                    x: parent.width/2 - width/2
                    y: parent.height/2 - height/2
                    width: parent.paintedWidth
                    height: parent.paintedHeight
                    onClicked: (mouse) => mouse.accepted = false
                }
            }
            Label {
                id: altMessage
                Layout.preferredWidth: image.paintedWidth
                Layout.alignment: Qt.AlignHCenter
                topPadding: 5
                leftPadding: 5
                rightPadding: 5
                bottomPadding: 5
                visible: text.length > 0
                wrapMode: Text.Wrap
                font.pointSize: AdjustedValues.f10
                text: model.index < imageFullView.alts.length ? imageFullView.alts[model.index] : ""
                background: Rectangle {
                    width: altMessage.width
                    height: altMessage.height
                    color: Material.backgroundColor
                }
            }
        }
    }

    RoundButton {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 5
        icon.source: "../images/close.png"
        onClicked: imageFullView.visible = false
    }

    MouseArea {
        anchors.fill: leftMoveButton
        onClicked: (mouse) => mouse.accepted = false
    }
    RoundButton {
        id: leftMoveButton
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 5
        enabled: imageFullListView.currentIndex > 0
        icon.source: "../images/arrow_back.png"
        onClicked: imageFullListView.decrementCurrentIndex()
    }
    MouseArea {
        anchors.fill: righttMoveButton
        onClicked: (mouse) => mouse.accepted = false
    }
    RoundButton {
        id: righttMoveButton
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 5
        enabled: imageFullListView.currentIndex < (imageFullListView.count - 1)
        icon.source: "../images/arrow_forward.png"
        onClicked: imageFullListView.incrementCurrentIndex()
    }
}
