import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.singleton 1.0

import "../controls"

Frame {
    id: imagePreviewLayout
    visible: embedImages.length > 0
    contentWidth: contentRootLayout.implicitWidth
    contentHeight: contentRootLayout.implicitHeight
    topInset: 0
    leftInset: 0
    rightInset: 0
    bottomInset: 0
    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    background: Rectangle {
        color: "transparent"
    }

    // 0:compact, 1:normal, 2:when one is whole, 3:all whole
    property int layoutType: 1
    property int layoutWidth: 100
    property var embedImages: []
    property var embedAlts: []
    property var embedImageRatios: []

    property int cellWidthAdjust: 3
    property int cellWidth: imagePreviewLayout.layoutWidth / contentRootLayout.columns - cellWidthAdjust

    signal requestViewImages(int index)

    states: [
        State {
            when: layoutType === 0
            PropertyChanges { target: contentRootLayout; columns: 4 }
            PropertyChanges { target: imagePreviewLayout; cellWidthAdjust: 5 }
        },
        State {
            when: layoutType === 1 || layoutType === 2
            PropertyChanges { target: contentRootLayout; columns: 2 }
            PropertyChanges { target: imagePreviewLayout; cellWidthAdjust: 3 }
        },
        State {
            when: layoutType === 3
            PropertyChanges { target: contentRootLayout; columns: 1 }
            PropertyChanges { target: imagePreviewLayout; cellWidthAdjust: 0 }
        }
    ]

    GridLayout {
        id: contentRootLayout
        columnSpacing: 6
        rowSpacing: 6
        columns: 2
        Repeater {
            id: repeater
            model: imagePreviewLayout.embedImages
            delegate: ImageWithIndicator {
                id: image
                property bool isWide: false
                property bool isTall: false
                Layout.preferredWidth: isWide ? imagePreviewLayout.layoutWidth : imagePreviewLayout.cellWidth
                Layout.preferredHeight: isTall ? (imagePreviewLayout.layoutWidth * (
                                                      model.index < embedImageRatios.length ? parseFloat(embedImageRatios[model.index]) : (sourceSize.height / sourceSize.width)
                                                      )
                                                  ) : imagePreviewLayout.cellWidth
                Layout.columnSpan: isWide ? 2 : 1
                fillMode: Image.PreserveAspectCrop
                source: modelData
                states: [
                    State {
                        when: imagePreviewLayout.layoutType === 0
                        PropertyChanges { target: image; isWide: false }
                        PropertyChanges { target: image; isTall: false }
                    },
                    State {
                        when: imagePreviewLayout.layoutType === 1
                        PropertyChanges { target: image; isWide: (repeater.count % 2 === 1 && model.index === (repeater.count - 1)) }
                        PropertyChanges { target: image; isTall: false }
                    },
                    State {
                        when: imagePreviewLayout.layoutType === 2
                        PropertyChanges { target: image; isWide: (repeater.count % 2 === 1 && model.index === (repeater.count - 1)) }
                        PropertyChanges { target: image; isTall: (repeater.count === 1) }
                    },
                    State {
                        when: imagePreviewLayout.layoutType === 3
                        PropertyChanges { target: image; isWide: false }
                        PropertyChanges { target: image; isTall: true }
                    }
                ]
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
}
