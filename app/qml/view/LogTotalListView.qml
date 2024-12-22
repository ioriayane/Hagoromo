import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

ScrollView {
    id: statisticsScrollView
    ScrollBar.vertical.policy: verticalScrollBar ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    clip: true

    property bool verticalScrollBar: false
    property int verticalScrollBarWidth: verticalScrollBar ? statisticsScrollView.ScrollBar.vertical.width : 0
    property alias model: statisticsListView.model
    property bool selectable: false

    signal clickedItem(string name)

    ListView {
        id: statisticsListView
        anchors.fill: parent
        currentIndex: -1
        delegate: ItemDelegate {
            width: statisticsListView.width - statisticsScrollView.verticalScrollBarWidth
            height: nameLabel.contentHeight * 2
            highlighted: (statisticsListView.currentIndex === model.index)
            Rectangle {
                visible: model.count > 0
                anchors.fill: parent
                anchors.leftMargin:  (1 - model.percent) * parent.width
                color: Material.color(Material.LightBlue)
                opacity: 0.2
            }
            RowLayout {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 5
                anchors.rightMargin: 5
                Label {
                    id: nameLabel
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    font.pointSize: AdjustedValues.f10
                    text: model.name
                }
                Label {
                    Layout.alignment: Qt.AlignVCenter
                    font.pointSize: AdjustedValues.f10
                    text: model.count
                }
            }
            onClicked: {
                if(statisticsScrollView.selectable === true){
                    statisticsListView.currentIndex = model.index
                }
                statisticsScrollView.clickedItem(model.name)
            }
        }
        section.property: "group"
        section.criteria: ViewSection.FullString
        section.delegate: Rectangle {
            width: statisticsListView.width - statisticsScrollView.verticalScrollBarWidth
            height: text.contentHeight * 1.3 * AdjustedValues.ratio
            color: Material.color(Material.BlueGrey)
            required property string section
            Text {
                id: text
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.verticalCenter: parent.verticalCenter
                text: parent.section
                font.bold: true
                font.pixelSize: AdjustedValues.f12
                color: "white"
            }
        }
    }
}
