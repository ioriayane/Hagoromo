import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

RowLayout {

    property alias source: icon.source
    property alias text: label.text

    Image {
        id: icon
        Layout.preferredWidth: label.contentHeight
        Layout.preferredHeight: label.contentHeight
    }
    Label {
        id: label
        font.pointSize: AdjustedValues.f8
    }
}
