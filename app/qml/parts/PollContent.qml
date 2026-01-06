import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import tech.relog.hagoromo.singleton 1.0

Frame {
    id: pollContent

    property var options: [] // str[]
    property var countOfOptions: [] // str[]
    property var indexOfOptions: [] // str[]
    property bool myPoll: false
    property string myVote: "-1"
    property int totalVotes: 0
    property bool isEnded: false
    property int remainTime: 0
    property bool running: false
    property string originalUrl: ""

    signal clicked(string vote_index)

    ButtonGroup {
        id: radioGroup
    }

    ColumnLayout {
        id: choicesLayout
        width: pollContent.width - pollContent.padding * 2

        property color borderColorNormal: Material.color(Material.Grey, Material.theme === Material.Dark ?
                                                             Material.Shade600 : Material.Shade400)
        property color colorSelected: Material.color(Material.accent, Material.theme === Material.Dark ?
                                                         Material.ShadeA100 : Material.Shade400)

        Repeater {
            model: pollContent.options
            delegate: RadioButton {
                id: control
                Layout.fillWidth: true
                indicator.visible: (pollContent.myVote === "-1" && !pollContent.isEnded)
                checked: false
                text: modelData
                ButtonGroup.group: radioGroup
                property string value: pollContent.indexOfOptions[model.index]
                property bool isVoted: pollContent.indexOfOptions[model.index] === pollContent.myVote

                topPadding: AdjustedValues.s10
                bottomPadding: AdjustedValues.s10

                contentItem: RowLayout {
                    width: control.width
                    Label {
                        Layout.leftMargin: control.indicator.x + control.indicator.width
                        Layout.fillWidth: true
                        font.pointSize: AdjustedValues.f10
                        text: control.text
                        enabled: true
                        elide: Text.ElideRight

                        Rectangle {
                            anchors.right: parent.right
                            width: votedLabel.contentWidth + height
                            height: votedLabel.height
                            visible: control.isVoted
                            color: Material.accentColor
                            radius: height * 0.5
                            Label {
                                id: votedLabel
                                anchors.centerIn: parent
                                font.pointSize: AdjustedValues.f8
                                text: qsTr("Voted")
                            }
                        }
                    }
                    Label {
                        visible: pollContent.myPoll || pollContent.myVote !== "-1"
                        font.pointSize: AdjustedValues.f10
                        text: pollContent.totalVotes === 0 ? "0%" : (100 * pollContent.countOfOptions[model.index] / pollContent.totalVotes).toFixed(0) + "%"
                    }
                }
                background: Rectangle {
                    color: Material.backgroundColor
                    border.color: control.isVoted ? Material.accentColor : choicesLayout.borderColorNormal
                    border.width: 1
                    radius: 2
                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: parent.width * pollContent.countOfOptions[model.index] / pollContent.totalVotes
                        color: Material.color(Material.accent)
                        opacity: 0.4
                        visible: pollContent.myPoll || pollContent.myVote !== "-1"
                        Behavior on width {
                            NumberAnimation { duration: 300; easing.type: Easing.OutBounce }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.topMargin: AdjustedValues.s5
            Layout.leftMargin: AdjustedValues.s10
            spacing: AdjustedValues.s10
            Label {
                color: Material.color(Material.Grey)
                font.pointSize: AdjustedValues.f8
                text: qsTr("%s votes").replace("%s", pollContent.totalVotes)
            }
            Label {
                Layout.fillWidth: true
                color: Material.color(Material.Grey)
                font.pointSize: AdjustedValues.f8
                text: pollContent.isEnded ? qsTr("Finished") : qsTr("%s hours left").replace("%s", pollContent.remainTime)

                Button {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    height: AdjustedValues.b36
                    visible: !pollContent.isEnded
                    enabled: !pollContent.running &&
                             (
                                 pollContent.myVote !== "-1" ||
                                 radioGroup.checkState !== Qt.Unchecked
                                 )
                    text: pollContent.myVote !== "-1" ? qsTr("Remove vote") : qsTr("Vote")
                    onClicked: {
                        if(pollContent.myVote === "-1"){
                            pollContent.clicked(radioGroup.checkedButton.value)
                        }else{
                            if(radioGroup.checkedButton){
                                radioGroup.checkedButton.checked = false
                            }
                            pollContent.clicked(pollContent.myVote)
                        }
                    }
                    BusyIndicator {
                        anchors.fill: parent
                        anchors.margins: 3
                        visible: pollContent.running
                    }
                }
            }
        }
        Label {
            Layout.leftMargin: AdjustedValues.s10
            color: Material.color(Material.Grey)
            font.pointSize: AdjustedValues.f8
            text: pollContent.originalUrl.length === 0 ?
                      pollContent.originalUrl :
                      "- <a href='%1'>Powered by TOKIMEKI</a> -".replace("%1", pollContent.originalUrl)
            onLinkActivated: (url) => Qt.openUrlExternally(url)
        }
    }
}
