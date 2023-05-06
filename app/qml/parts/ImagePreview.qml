import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

GridLayout {
    id: imagePreviewLayout
    visible: embedImages.length > 0
    columnSpacing: 6
    rowSpacing: 6
    columns: 2

    property int layoutWidth: 100
    property string embedImages: model.embedImages

    property int cellWidth: imagePreviewLayout.layoutWidth * 0.5 - 3

    Repeater {
        id: repeater
        model: imagePreviewLayout.embedImages.split("\n")
        delegate: Image {
            property bool isWide: (repeater.count % 2 === 1 && model.index === (repeater.count - 1))
            Layout.preferredWidth: isWide ? imagePreviewLayout.layoutWidth : imagePreviewLayout.cellWidth
            Layout.preferredHeight: imagePreviewLayout.cellWidth
            Layout.columnSpan: isWide ? 2 : 1
            fillMode: Image.PreserveAspectCrop
            source: modelData
        }
    }
}