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

    signal clickedItem(string name)

    ListView {
        id: statisticsListView
        anchors.fill: parent
        delegate: ItemDelegate {
            width: statisticsListView.width - statisticsScrollView.verticalScrollBarWidth
            height: nameLabel.contentHeight * 2
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
            onClicked: statisticsScrollView.clickedItem(model.name)
        }
    }
}
