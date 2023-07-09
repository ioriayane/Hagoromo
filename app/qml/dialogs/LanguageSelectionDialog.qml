import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.languagelistmodel 1.0

Dialog {
    id: languageSelectionDialog
    modal: true
    x: (parent.width - width) * 0.5
    y: (parent.height - height) * 0.5
    title: qsTr("Select language")

    property variant selectedLanguages: []

    function setSelectedLanguages(langs){
        console.log("setSelectedLanguages:" + langs)
        languageListModel.setSelectedLanguages(langs)
    }

    ColumnLayout {
        ScrollView {
            id: languageScroll
            Layout.preferredWidth: 300
            Layout.preferredHeight: 300
            //ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            clip: true

            ListView {
                id: languageList

                model: LanguageListModel {
                    id: languageListModel
                }

                delegate: CheckDelegate {
                    id: checkDelegate
                    width: languageScroll.width - languageScroll.ScrollBar.vertical.width
                    text: model.name
                    checked: model.checked
                    enabled: model.enabled
                    onClicked: {
                        languageListModel.update(index, LanguageListModel.CheckedRole, checkDelegate.checked)
                        selectedLanguages = languageListModel.selectedLanguages()
                    }
                }
            }
        }
        RowLayout {
            Button {
                text: qsTr("Cancel")
                flat: true
                onClicked: languageSelectionDialog.reject()
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
            }
            Button {
                text: qsTr("OK")
                onClicked: languageSelectionDialog.accept()
            }
        }
    }
}

