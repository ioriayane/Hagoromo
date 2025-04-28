import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.singleton 1.0

import "../compat"

RowLayout {
    id: headerLayout
    clip: true

    property int layoutWidth: 300
    property string displayName: ""
    property string handle: ""
    property string indexedAt: ""
    property string verificationState: "none"

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
        Layout.alignment: Qt.AlignBottom
        elide: Text.ElideRight
        opacity: 0.8
        font.pointSize: AdjustedValues.f8
        text: headerLayout.handle
    }
    Image {
        Layout.preferredWidth: handleText.height
        Layout.preferredHeight: handleText.height
        fillMode: Image.PreserveAspectFit
        visible: (headerLayout.verificationState !== "none")
        source: headerLayout.verificationState === "verifier" ?
                    "../images/verifier.png" : "../images/verified.png"
        layer.enabled: true
        layer.effect: ColorOverlayC {
            color: Material.color(Material.Blue)
        }
    }
    Item {
        id: spacerLine1Item
        Layout.fillWidth: true
        Layout.preferredHeight: 1
    }
    Label {
        id: indexAtText
        Layout.alignment: Qt.AlignBottom
        Layout.minimumWidth: contentWidth
        opacity: 0.8
        font.pointSize: AdjustedValues.f8
        text: headerLayout.indexedAt
    }
}
