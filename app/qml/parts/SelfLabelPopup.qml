import QtQuick 2.15
import QtQuick.Controls 2.15

import "../controls"

MenuEx {
    id: selfLabelPopup

    signal triggered(string value, string text)

    Action {
        text: qsTr("Sexually Explicit")
        property string value: "porn"
        onTriggered: selfLabelPopup.triggered(value, text)
    }
    Action {
        text: qsTr("Nudity")
        property string value: "nudity"
        onTriggered: selfLabelPopup.triggered(value, text)
    }
    Action {
        text: qsTr("Sexually Suggestive")
        property string value: "sexual"
        onTriggered: selfLabelPopup.triggered(value, text)
    }
    Action {
        text: qsTr("Gore")
        property string value: "graphic-media"
        onTriggered: selfLabelPopup.triggered(value, text)
    }
    MenuSeparator { }
    Action {
        text: qsTr("Remove")
        property string value: ""
        onTriggered: selfLabelPopup.triggered(value, text)
    }
}
