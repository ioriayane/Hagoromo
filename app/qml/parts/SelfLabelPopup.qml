import QtQuick 2.15
import QtQuick.Controls 2.15

Menu {
    id: selfLabelPopup

    signal triggered(string value, string text)

    MenuItem {
        text: qsTr("Sexually Explicit")
        property string value: "porn"
        onTriggered: selfLabelPopup.triggered(value, text)
    }
    MenuItem {
        text: qsTr("Nudity")
        property string value: "nudity"
        onTriggered: selfLabelPopup.triggered(value, text)
    }
    MenuItem {
        text: qsTr("Sexually Suggestive")
        property string value: "sexual"
        onTriggered: selfLabelPopup.triggered(value, text)
    }
    MenuItem {
        text: qsTr("NSFL")
        property string value: "nsfl"
        onTriggered: selfLabelPopup.triggered(value, text)
    }
    MenuItem {
        text: qsTr("Gore")
        property string value: "gore"
        onTriggered: selfLabelPopup.triggered(value, text)
    }
    MenuSeparator {
        topPadding: 1
        bottomPadding: 1
    }
    MenuItem {
        text: qsTr("Spoiler")
        property string value: "spoiler"
        onTriggered: selfLabelPopup.triggered(value, text)
    }
    MenuItem {
        text: qsTr("Content warning")
        property string value: "!warn"
        onTriggered: selfLabelPopup.triggered(value, text)
    }
    MenuSeparator {
        topPadding: 1
        bottomPadding: 1
    }
    MenuItem {
        text: qsTr("Remove")
        property string value: ""
        onTriggered: selfLabelPopup.triggered(value, text)
    }
}
