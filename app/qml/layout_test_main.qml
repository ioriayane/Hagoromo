import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "controls"
import "data"
import "dialogs"
import "view"
import "parts"
import "compat"

ApplicationWindow {
    id: layoutTestWindow
    width: 900
    height: 700
    visible: true
    title: "Hagoromo - Layout Test Mode"

    Material.theme: Material.Light
    Material.accent: Material.Blue

    QtObject {
        id: translatorChanger
        objectName: "translatorChanger"
        signal triggered(string lang)
    }

    SettingDialog {
        id: settingDialog
        x: parent.width / 2 - width / 2
        // y: sideBarItem.height / 2 - height / 2
        onAccepted: {
            repeater.updateSettings(2)
        }
    }

    Button {
        text: "open"
        onClicked: selectDraftDialog.open()
    }

    SelectDraftDialog {
        id: selectDraftDialog
    }

}
