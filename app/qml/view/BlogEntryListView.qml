import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "../parts"
import "../controls"
import "../compat"

ScrollView {
    id: blogScrollView
    ScrollBar.vertical.policy: ScrollBar.AlwaysOff

    property string hoveredLink: ""

    property alias model: blogListView.model
    property alias currentItem: blogListView.currentItem
    property alias count: blogListView.count

    ListView {
        id: blogListView
        orientation:ListView.Horizontal
        clip: true
        spacing: 2
        onCountChanged: positionViewAtBeginning()
        delegate: ClickableFrame {
            id: blogEntryFrame
            clip: true
            hoverEnabled: true
            contentWidth: contentRootLayout.implicitWidth
            contentHeight: contentRootLayout.implicitHeight

            ColumnLayout {
                id: contentRootLayout
                property int basisWidth: blogListView.width * 0.8 -
                                         blogEntryFrame.leftPadding - blogEntryFrame.rightPadding
                RowLayout {
                    id: blogEntryServiceLayout
                    Image {
                        Layout.preferredWidth: AdjustedValues.i12
                        Layout.preferredHeight: AdjustedValues.i12
                        Layout.alignment: Qt.AlignBaseline
                        layer.enabled: true
                        layer.effect: ColorOverlayC {
                            color: Material.color(Material.Blue)
                        }
                        source: "../images/open_in_other.png"
                    }

                    Label {
                        font.pointSize: AdjustedValues.f8
                        text: model.serviceName
                        color: Material.color(Material.Blue)
                    }
                    Label {
                        visible: model.visibility.length > 0
                        font.pointSize: AdjustedValues.f8
                        text: "[" + model.visibility + "]"
                        color: Material.color(Material.Yellow)
                    }
                }
                Label {
                    id: blogEntryTitleLabel
                    font.pointSize: AdjustedValues.f10
                    text: model.title
                }
                RowLayout {
                    Layout.preferredWidth: blogEntryServiceLayout.width > blogEntryTitleLabel.width ?
                                               blogEntryServiceLayout.width : blogEntryTitleLabel.width
                    Label {
                        Layout.fillWidth: true
                        font.pointSize: AdjustedValues.f8
                        elide: Text.ElideRight
                        color: Material.color(Material.Grey)
                        text: model.content.split("\n")[0]
                    }
                    Label {
                        Layout.minimumWidth: contentWidth
                        font.pointSize: AdjustedValues.f8
                        color: Material.color(Material.Grey)
                        text: model.createdAt
                    }
                }
            }
            onClicked: (mouse) => Qt.openUrlExternally(model.permalink)
            onHoveredChanged: {
                if(hovered){
                    blogScrollView.hoveredLink = model.permalink
                }else{
                    blogScrollView.hoveredLink = ""
                }
            }
        }
    }
}
