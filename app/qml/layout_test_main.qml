import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.accountlistmodel 1.0

import tech.relog.hagoromo.singleton 1.0

import "controls"
import "data"
import "dialogs"
import "view"
import "parts"
import "compat"

ApplicationWindow {
    id: layoutTestWindow
    width: 1300
    height: 1300
    visible: true
    title: "Hagoromo - Layout Test Mode"

    Material.theme: Material.Light
    // Material.theme: Material.Dark
    Material.accent: Material.Blue

    QtObject {
        id: translatorChanger
        objectName: "translatorChanger"
        signal triggered(string lang)
    }

    // SettingDialog {
    //     id: settingDialog
    //     x: parent.width / 2 - width / 2
    //     // y: sideBarItem.height / 2 - height / 2
    //     onAccepted: {
    //         repeater.updateSettings(2)
    //     }
    // }

    // Button {
    //     text: "open"
    //     // onClicked: selectDraftDialog.open()
    //     onClicked: postDialog.open()
    // }

    // Component.onCompleted: {
    //     accountListModel.load()
    // }
    // AccountListModel {
    //     id: accountListModel
    //     onFinished: {
    //         console.log("!!!!!! Finish account load. !!!!!!")
    //     }
    // }
    // PostDialog {
    //     id: postDialog
    //     accountModel: accountListModel
    // }

    // SelectDraftDialog {
    //     id: selectDraftDialog
    // }

    ComboBox {
        id: layoutTypeComboBox
        x: 10
        y: 10
        model: ["Type 0", "Type 1", "Type 2", "Type 3"]
    }
    GridLayout {
        anchors.top: layoutTypeComboBox.bottom
        anchors.topMargin: 10
        width: 1300
        // spacing: 12
        columnSpacing: 6
        rowSpacing: 6
        columns: 4
        property int layoutType: layoutTypeComboBox.currentIndex
        property int cellWidth: 300
        ImagePreview {
            Layout.preferredWidth: parent.cellWidth
            // height: 100
            layoutWidth: width

            layoutType: parent.layoutType
            embedImages: [
                "../images/expand_less.png"
                // , "../images/delete.png"
                // , "../images/video.png"
                // , "../images/tag.png"
                // , "../images/label.png"
                // , "../images/quote.png"
                // , "../images/reply.png"
                // , "../images/report.png"
                // , "../images/search.png"
                // , "../images/send.png"
            ]
            embedAlts: [""]
            embedImageRatios: ["1"]
            Rectangle {
                anchors.fill: parent
                border.color: "blue"
                border.width: 1
                color: "transparent"
            }
        }
        ImagePreview {
            Layout.preferredWidth: parent.cellWidth  
            // height: 100
            layoutWidth: width

            layoutType: parent.layoutType
            embedImages: [
                "../images/expand_less.png"
                , "../images/delete.png"
                // , "../images/video.png"
                // , "../images/tag.png"
                // , "../images/label.png"
                // , "../images/quote.png"
                // , "../images/reply.png"
                // , "../images/report.png"
                // , "../images/search.png"
                // , "../images/send.png"
            ]
            embedAlts: ["", ""]
            embedImageRatios: ["1", "1"]
            Rectangle {
                anchors.fill: parent
                border.color: "blue"
                border.width: 1
                color: "transparent"
            }
        }
        ImagePreview {
            Layout.preferredWidth: parent.cellWidth
            // height: 100
            layoutWidth: width

            layoutType: parent.layoutType
            embedImages: [
                "../images/expand_less.png"
                , "../images/delete.png"
                , "../images/video.png"
                // , "../images/tag.png"
                // , "../images/label.png"
                // , "../images/quote.png"
                // , "../images/reply.png"
                // , "../images/report.png"
                // , "../images/search.png"
                // , "../images/send.png"
            ]
            embedAlts: ["", "" , ""]
            embedImageRatios: ["1", "1", "1"]
            Rectangle {
                anchors.fill: parent
                border.color: "blue"
                border.width: 1
                color: "transparent"
            }
        }
        ImagePreview {
            Layout.preferredWidth: parent.cellWidth
            // height: 100
            layoutWidth: width

            layoutType: parent.layoutType
            embedImages: [
                "../images/expand_less.png"
                , "../images/delete.png"
                , "../images/video.png"
                , "../images/tag.png"
                // , "../images/label.png"
                // , "../images/quote.png"
                // , "../images/reply.png"
                // , "../images/report.png"
                // , "../images/search.png"
                // , "../images/send.png"
            ]
            embedAlts: ["", "" , "", ""]
            embedImageRatios: ["1", "1", "1", "1"]
            Rectangle {
                anchors.fill: parent
                border.color: "blue"
                border.width: 1
                color: "transparent"
            }
        }
        ImagePreview {
            Layout.preferredWidth: parent.cellWidth
            // height: 100
            layoutWidth: width

            layoutType: parent.layoutType
            embedImages: [
                "../images/expand_less.png"
                , "../images/delete.png"
                , "../images/video.png"
                , "../images/tag.png"
                , "../images/label.png"
                // , "../images/quote.png"
                // , "../images/reply.png"
                // , "../images/report.png"
                // , "../images/search.png"
                // , "../images/send.png"
            ]
            embedAlts: ["", "" , "" , "" , ""]
            embedImageRatios: ["1", "1", "1", "1", "1"]
            Rectangle {
                anchors.fill: parent
                border.color: "blue"
                border.width: 1
                color: "transparent"
            }
        }
        ImagePreview {
            Layout.preferredWidth: parent.cellWidth
            // height: 100
            layoutWidth: width

            layoutType: parent.layoutType
            embedImages: [
                "../images/expand_less.png"
                , "../images/delete.png"
                , "../images/video.png"
                , "../images/tag.png"
                , "../images/label.png"
                , "../images/quote.png"
                // , "../images/reply.png"
                // , "../images/report.png"
                // , "../images/search.png"
                // , "../images/send.png"
            ]
            embedAlts: ["", "", "", "", "", "", ""]
            embedImageRatios: ["1", "1", "1", "1", "1", "1", "1"]
            Rectangle {
                anchors.fill: parent
                border.color: "blue"
                border.width: 1
                color: "transparent"
            }
        }
        ImagePreview {
            Layout.preferredWidth: parent.cellWidth
            // height: 100
            layoutWidth: width

            layoutType: parent.layoutType
            embedImages: [
                "../images/expand_less.png"
                , "../images/delete.png"
                , "../images/video.png"
                , "../images/tag.png"
                , "../images/label.png"
                , "../images/quote.png"
                , "../images/reply.png"
                // , "../images/report.png"
                // , "../images/search.png"
                // , "../images/send.png"
            ]
            embedAlts: ["", "", "", "", "", "", ""]
            embedImageRatios: ["1", "1", "1", "1", "1", "1", "1"]
            Rectangle {
                anchors.fill: parent
                border.color: "blue"
                border.width: 1
                color: "transparent"
            }
        }
        ImagePreview {
            Layout.preferredWidth: parent.cellWidth
            // height: 100
            layoutWidth: width

            layoutType: parent.layoutType
            embedImages: [
                "../images/expand_less.png"
                , "../images/delete.png"
                , "../images/video.png"
                , "../images/tag.png"
                , "../images/label.png"
                , "../images/quote.png"
                , "../images/reply.png"
                , "../images/report.png"
                // , "../images/search.png"
                // , "../images/send.png"
            ]
            embedAlts: ["", "", "", "", "", "", ""]
            embedImageRatios: ["1", "1", "1", "1", "1", "1", "1"]
            Rectangle {
                anchors.fill: parent
                border.color: "blue"
                border.width: 1
                color: "transparent"
            }
        }
        ImagePreview {
            Layout.preferredWidth: parent.cellWidth
            // height: 100
            layoutWidth: width

            layoutType: parent.layoutType
            embedImages: [
                "../images/expand_less.png"
                , "../images/delete.png"
                , "../images/video.png"
                , "../images/tag.png"
                , "../images/label.png"
                , "../images/quote.png"
                , "../images/reply.png"
                , "../images/report.png"
                , "../images/search.png"
                // , "../images/send.png"
            ]
            embedAlts: ["", "", "", "", "", "", "", "", "", ""]
            embedImageRatios: ["1", "1", "1", "1", "1", "1", "1", "1", "1", "1"]
            Rectangle {
                anchors.fill: parent
                border.color: "blue"
                border.width: 1
                color: "transparent"
            }
        }
        ImagePreview {
            Layout.preferredWidth: parent.cellWidth
            // height: 100
            layoutWidth: width

            layoutType: parent.layoutType
            embedImages: [
                "../images/expand_less.png"
                , "../images/delete.png"
                , "../images/video.png"
                , "../images/tag.png"
                , "../images/label.png"
                , "../images/quote.png"
                , "../images/reply.png"
                , "../images/report.png"
                , "../images/search.png"
                , "../images/send.png"
            ]
            embedAlts: ["", "", "", "", "", "", "", "", "", ""]
            embedImageRatios: ["1", "1", "1", "1", "1", "1", "1", "1", "1", "1"]
            Rectangle {
                anchors.fill: parent
                border.color: "blue"
                border.width: 1
                color: "transparent"
            }
        }

    }
}
