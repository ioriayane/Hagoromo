import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

ColumnLayout {
    spacing: 0
    property alias tagsLayout: tagsLayout
    property alias labelsLayout: labelsLayout
    property alias languagesLayout: languagesLayout
    property alias indexedAtLongLabel: indexedAtLongLabel
    property alias viaTagLabel: viaTagLabel

    TagLabelLayout {
        id: tagsLayout
        Layout.preferredWidth: parent.width
        Layout.topMargin: 5
        visible: count > 0
        iconSource: "../images/tag.png"
        tagSpacing: 5
    }
    TagLabelLayout {
        id: labelsLayout
        Layout.preferredWidth: parent.width
        visible: count > 0
        tagSpacing: 2
    }
    TagLabelLayout {
        id: languagesLayout
        Layout.preferredWidth: parent.width
        Layout.topMargin: 5
        visible: count > 0
        iconSource: "../images/language.png"
        tagSpacing: 5
    }

    Label {
        id: indexedAtLongLabel
        Layout.topMargin: 5
        visible: text.length > 0
        font.pointSize: AdjustedValues.f8
        color: Material.color(Material.Grey)
    }
    Label {
        id: viaTagLabel
        Layout.topMargin: 3
        visible: text.length > 0
        font.pointSize: AdjustedValues.f8
        color: Material.color(Material.Grey)
    }
}
