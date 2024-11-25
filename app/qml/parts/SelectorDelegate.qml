import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import tech.relog.hagoromo.singleton 1.0

import "../controls"
import "../parts"

RowLayout {
    id: selectorDelegate
    spacing: 0

    property int indent: 0
    property int index: -1
    property string type: ""
    property string displayType: ""
    property bool highlighted: false
    property bool useAppendButton: false

    signal appendChild(int row, string type)
    signal remove(int row)
    signal clicked()

    Item {
        Layout.preferredWidth: AdjustedValues.s20 * selectorDelegate.indent
        Layout.preferredHeight: 1
    }
    Rectangle {
        Layout.preferredWidth: 1 //AdjustedValues.s5
        Layout.preferredHeight: selectorItem.height * 0.8
        Layout.alignment: Qt.AlignVCenter
        opacity: 0.5
        color: Material.foreground
    }
    ItemDelegate {
        id: selectorItem
        Layout.preferredHeight: AdjustedValues.b36
        text: selectorDelegate.displayType
        highlighted: selectorDelegate.highlighted
        font.pointSize: AdjustedValues.f10
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: (mouse) => {
                           if(mouse.button === Qt.RightButton){
                               itemRightMenu.x = mouse.x
                               itemRightMenu.y = mouse.y
                               itemRightMenu.open()
                           }else{
                               if(selectorDelegate.useAppendButton){
                                   itemLeftMenu.x = mouse.x
                                   itemLeftMenu.y = mouse.y
                                   itemLeftMenu.open()
                               }else{
                                   selectorDelegate.clicked()
                               }
                           }
                       }
        }

        Rectangle {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: AdjustedValues.i16
            height: AdjustedValues.i16
            radius: width / 2
            border.width: 1
            border.color: Material.foreground
            visible: selectorDelegate.useAppendButton
            Image {
                anchors.fill: parent
                source: "../images/add.png"
            }
        }
        MenuEx {
            id: itemLeftMenu
            Action {
                text: qsTr("AND")
                onTriggered: selectorDelegate.appendChild(selectorDelegate.index, "and")
            }
            Action {
                text: qsTr("OR")
                onTriggered: selectorDelegate.appendChild(selectorDelegate.index, "or")
            }
            // Action {
            //     text: qsTr("NOT")
            //     onTriggered: selectorDelegate.appendChild(selectorDelegate.index, "not")
            // }
            Action {
                text: qsTr("XOR")
                onTriggered: selectorDelegate.appendChild(selectorDelegate.index, "xor")
            }
            MenuSeparator {}
            Action {
                text: qsTr("Following")
                onTriggered: selectorDelegate.appendChild(selectorDelegate.index, "following")
            }
            Action {
                text: qsTr("Followers")
                onTriggered: selectorDelegate.appendChild(selectorDelegate.index, "followers")
            }
            Action {
                text: qsTr("List")
                onTriggered: selectorDelegate.appendChild(selectorDelegate.index, "list")
            }
            Action {
                text: qsTr("Me")
                onTriggered: selectorDelegate.appendChild(selectorDelegate.index, "me")
            }
        }
        MenuEx {
            id: itemRightMenu
            Action {
                text: qsTr("Remove")
                onTriggered: selectorDelegate.remove(selectorDelegate.index)
            }
        }
    }
}
