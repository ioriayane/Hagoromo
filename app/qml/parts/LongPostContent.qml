import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

ColumnLayout {
    id: longPostContent
    spacing: 0
    visible: active && (!loaded || edited)

    property bool active: false
    property bool loaded: false
    property bool running: false
    property bool failed: false
    property bool edited: false
    property string articleUrl: ""

    signal requestGetArticle()

    Label {
        Layout.fillWidth: true
        visible: longPostContent.loaded && longPostContent.edited
        wrapMode: Text.Wrap
        font.pointSize: AdjustedValues.f8
        color: Material.color(Material.Grey)
        text: qsTr("Edited after posting")
    }
    Button {
        Layout.alignment: Qt.AlignRight
        Layout.preferredHeight: AdjustedValues.b36
        visible: !longPostContent.loaded
        enabled: !longPostContent.running
        text: longPostContent.failed ? qsTr("Read on mochott") : qsTr("Show more")
        onClicked: {
            if (longPostContent.failed) {
                Qt.openUrlExternally(longPostContent.articleUrl)
            } else {
                longPostContent.requestGetArticle()
            }
        }
        BusyIndicator {
            anchors.fill: parent
            visible: longPostContent.running
        }
    }
}
