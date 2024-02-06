import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import tech.relog.hagoromo.singleton 1.0

RowLayout {
    //anchors.fill: parent
    //anchors.margins: 10
    spacing: 5

    property alias source: avatarImage.source
    property alias handle: handleText.text
    property int leftMargin: 0

    AvatarImage {
        id: avatarImage
        Layout.preferredWidth: AdjustedValues.i24
        Layout.preferredHeight: AdjustedValues.i24
        Layout.leftMargin: leftMargin
    }
    Label {
        id: handleText
        elide: Text.ElideRight
        font.pointSize: AdjustedValues.f10
    }
    Item {
        Layout.fillWidth: true
        Layout.preferredHeight: 1
    }
}
