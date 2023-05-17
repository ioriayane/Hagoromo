import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

Frame {
    id: clickableFrame

    signal clicked(var mouse)

    background: MouseArea {
        Rectangle {
            anchors.fill: parent
            border.width: 1
            border.color: Material.color(Material.Grey, Material.Shade600)
            color: "transparent"
            radius: 2
        }

        onClicked: (mouse) => clickableFrame.clicked(mouse)
    }
}
