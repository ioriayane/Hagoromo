import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import "../controls"

IconLabelFrame {
    backgroundColor: Material.color(Material.Grey)
    borderWidth: 0
    iconSource: showContent ? "../images/visibility_on.png" : "../images/visibility_off.png"
    controlButton.visible: true
    controlButton.iconText: showContent ? qsTr("Hide") : qsTr("Show")
    controlButton.onClicked: showContent = !showContent
    visible: false
    onVisibleChanged: {
        if(visible){
            showContent = false
        }else{
            showContent = true
        }
    }
    property bool showContent: true
}
