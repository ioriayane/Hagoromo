import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.singleton 1.0

Frame {
    id: pollLayout

    property var options: [] // str[]
    property var countOfOptions: [] // str[]
    property var indexOfOptions: [] // str[]
    property string myVote: "-1"
    property int totalVotes: 0
    property bool isEnded: false
    property int remainTime: 0

    signal clicked(string vote_index)

    ButtonGroup {
        id: radioGroup
    }

    ColumnLayout {
        id: choicesLayout
        width: pollLayout.width - pollLayout.padding * 2

        property color borderColorNormal: Material.color(Material.Grey, Material.theme === Material.Dark ?
                                                             Material.Shade600 : Material.Shade400)
        property color borderColorSelected: Material.color(Material.Grey, Material.theme === Material.Dark ?
                                                               Material.Shade600 : Material.Shade400)

        Repeater {
            model: pollLayout.options
            // model: ["item1", "item2"]
            delegate: RadioButton {
                id: control
                Layout.fillWidth: true
                // enabled: pollLayout.myVote < 0
                indicator.visible: (pollLayout.myVote < 0 && !pollLayout.isEnded)
                checked: false
                text: modelData
                ButtonGroup.group: radioGroup
                property string value: pollLayout.indexOfOptions[model.index]

                contentItem: RowLayout {
                    Label {
                        Layout.leftMargin: control.indicator.x + control.indicator.width
                        font.pointSize: AdjustedValues.f10
                        text: control.text
                        enabled: true
                    }
                    Rectangle {
                        width: votedLabel.contentWidth + height
                        height: votedLabel.height
                        visible: pollLayout.indexOfOptions[model.index] === pollLayout.myVote
                        color: Material.accentColor
                        radius: height * 0.5
                        Label {
                            id: votedLabel
                            anchors.centerIn: parent
                            font.pointSize: AdjustedValues.f8
                            text: qsTr("Voted")
                        }
                    }
                    Control {
                        font.pointSize: AdjustedValues.f8
                        Layout.fillWidth: true
                    }
                    Label {
                        text: pollLayout.totalVotes === 0 ? "0%" : (100 * pollLayout.countOfOptions[model.index] / pollLayout.totalVotes).toFixed(0) + "%"
                    }
                }
                background: Rectangle {
                    color: Material.backgroundColor
                    border.color: pollLayout.indexOfOptions[model.index] === pollLayout.myVote ? Material.accentColor : choicesLayout.borderColorNormal
                    border.width: 1
                    radius: 2
                }
            }
        }

        RowLayout {
            Layout.leftMargin: AdjustedValues.s10
            spacing: AdjustedValues.s10
            Label {
                color: Material.color(Material.Grey)
                font.pointSize: AdjustedValues.f8
                text: qsTr("%s votes").replace("%s", pollLayout.totalVotes)
            }
            Label {
                color: Material.color(Material.Grey)
                font.pointSize: AdjustedValues.f8
                text: pollLayout.isEnded ? qsTr("Finished") : qsTr("%s hours left").replace("%s", pollLayout.remainTime)
            }

            Control {
                Layout.fillWidth: true
            }

            Button {
                enabled: !pollLayout.isEnded && radioGroup.checkState !== Qt.Unchecked
                text: pollLayout.myVote >= 0 ? qsTr("Remove vote") : qsTr("Vote")
                onClicked: pollLayout.clicked(radioGroup.checkedButton.value)
            }
        }
    }
}
