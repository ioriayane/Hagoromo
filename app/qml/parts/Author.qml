import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.singleton 1.0

RowLayout {
    id: headerLayout
    clip: true

    property int layoutWidth: 300
    property string displayName: ""
    property string handle: ""
    property string indexedAt: ""

    Label {
        id: displayNameText
        Layout.maximumWidth: layoutWidth / 2
        Layout.alignment: Qt.AlignTop
        font.pointSize: AdjustedValues.f10
        elide: Text.ElideRight
        text: headerLayout.displayName
    }
    Label {
        id: handleText
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop
        elide: Text.ElideRight
        opacity: 0.8
        font.pointSize: AdjustedValues.f8
        text: headerLayout.handle
    }
    Item {
        id: spacerLine1Item
        Layout.fillWidth: true
    }
    Label {
        id: indexAtText
        Layout.alignment: Qt.AlignTop
        Layout.minimumWidth: contentWidth
        opacity: 0.8
        font.pointSize: AdjustedValues.f8
        text: headerLayout.indexedAt
    }
}
