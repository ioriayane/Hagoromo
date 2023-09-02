import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.singleton 1.0

import "../controls"

GridLayout {
    id: imagePreviewLayout
    visible: embedImages.length > 0
    columnSpacing: 6
    rowSpacing: 6
    columns: 2

    property int layoutWidth: 100
    property var embedImages: []
    property var embedAlts: []

    property int cellWidth: imagePreviewLayout.layoutWidth * 0.5 - 3

    signal requestViewImages(int index)

    Repeater {
        id: repeater
        model: imagePreviewLayout.embedImages
        delegate: ImageWithIndicator {
            property bool isWide: (repeater.count % 2 === 1 && model.index === (repeater.count - 1))
            Layout.preferredWidth: isWide ? imagePreviewLayout.layoutWidth : imagePreviewLayout.cellWidth
            Layout.preferredHeight: imagePreviewLayout.cellWidth
            Layout.columnSpan: isWide ? 2 : 1
            fillMode: Image.PreserveAspectCrop
            source: modelData
            TagLabel {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: 3
                visible: model.index < embedAlts.length ? embedAlts[model.index].length > 0 : false
                source: ""
                fontPointSize: AdjustedValues.f8
                text: "Alt"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: imagePreviewLayout.requestViewImages(model.index)
            }
        }
    }
}
