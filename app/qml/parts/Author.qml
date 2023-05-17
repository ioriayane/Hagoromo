import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

RowLayout {
    id: headerLayout
    clip: true

    property string displayName: ""
    property string handle: ""
    property string indexedAt: ""

    Label {
        Layout.maximumWidth: headerLayout.width - handleText.width - indexAtText.width - headerLayout.spacing * 3
        font.pointSize: 10
        elide: Text.ElideRight
        text: headerLayout.displayName
    }
    Label {
        id: handleText
        opacity: 0.8
        font.pointSize: 8
        text: headerLayout.handle
    }
    Item {
        id: spacerLine1Item
        Layout.fillWidth: true
    }
    Label {
        id: indexAtText
        Layout.minimumWidth: contentWidth
        opacity: 0.8
        font.pointSize: 8
        text: headerLayout.indexedAt
    }
}
