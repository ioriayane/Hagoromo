import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.singleton 1.0

RowLayout {
    id: knownFollowersInfo

    property alias model: repeater.model
    property string knownName: ""
    property string othersCount: "0"

    signal clicked()

    Repeater {
        id: repeater
        AvatarImage {
            Layout.preferredWidth: AdjustedValues.i16
            Layout.preferredHeight: AdjustedValues.i16
            source: model.modelData
        }
    }

    Label {
        Layout.fillWidth: true
        font.pointSize: AdjustedValues.f8
        font.underline: true
        text: qsTr("Followed by %1 and %2 others").replace("%1", knownName).replace("%2", othersCount)

        MouseArea {
            anchors.fill: parent
            onClicked: knownFollowersInfo.clicked()
        }
    }
}
